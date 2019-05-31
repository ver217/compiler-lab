%locations
%{
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "def.h"
#include "util.h"
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int yylex();
void yyerror(const char* fmt, ...);
void display(struct node *,int);
%}

%union {
    int type_int;
    float type_float;
    char type_id[32];
    char type_char;
    struct node *ptr;
};

//  %type 定义非终结符的语义值类型
%type <ptr> Program ExtStmtList ExtStmt Specifier Var VarDec VarDecList VarDecStmt ParamDec ParamList FuncDec FuncDecStmt Exp ArgList Stmt BlockStmt FuncDef
%type <ptr> BlockInnerStmtList
//% token 定义终结符的语义值类型
%token <type_int> INT              //指定INT的语义值是type_int，有词法分析得到的数值
%token <type_id> ID TYPE CMP  //指定ID,CMP 的语义值是type_id，有词法分析得到的标识符字符串
%token <type_float> FLOAT         //指定ID的语义值是type_id，有词法分析得到的标识符字符串
%token <type_char> CHAR

%token AND OR NOT IF ELSE WHILE RETURN
%token PLUS MINUS STAR DIV ASSIGNOP
%token INC DEC

%right ASSIGNOP
%left OR
%left AND
%left CMP
%left PLUS MINUS
%left STAR DIV
%right UMINUS NOT INC DEC

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%%
Program: ExtStmtList { display($1,0); /*semantic_Analysis0($1);*/}     /*显示语法树,语义分析*/
; 
ExtStmtList: { $$ = NULL; }
    | ExtStmt ExtStmtList { $$ = mknode(EXT_DEF_LIST, $1, $2, NULL, yylineno); }   //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数
;  
ExtStmt: VarDecStmt { $$ = $1; $$->kind = EXT_VAR_DEF; }   //该结点对应一个外部变量声明
    | FuncDecStmt { $$ = $1; }         //TODO: UPDATE该结点对应一个函数定义
    | FuncDef { $$ = $1; }
    | error ';' { $$ = NULL; }
;
Specifier: TYPE {$$ = mknode(TYPE, NULL, NULL, NULL, yylineno); $$->type = resolve_type($1); strcpy($$->type_id, $1); }
;
Var: ID {$$ = mknode(ID, NULL, NULL, NULL, yylineno); strcpy($$->type_id, $1); }
;
VarDec: Var { $$ = $1; }
    | Var ASSIGNOP Exp { $$ = mknode(ASSIGNOP, $1, $3, NULL, yylineno); strcpy($$->type_id, "'='"); }
;
VarDecList: VarDec { $$ = mknode(DEC_LIST, $1, NULL, NULL, yylineno); }
    | VarDec ',' VarDecList { $$ = mknode(DEC_LIST, $1, $3, NULL, yylineno); }
;
VarDecStmt: Specifier VarDecList ';' {$$ = mknode(VAR_DEF, $1, $2, NULL, yylineno); }
;
ParamDec: Specifier Var { $$ = mknode(PARAM_DEC, $1, $2, NULL, yylineno); }
;
ParamList: ParamDec { $$ = mknode(PARAM_LIST, $1, NULL, NULL, yylineno); }
    | ParamDec ',' ParamList { $$ = mknode(PARAM_LIST, $1, $3, NULL, yylineno); }
;
FuncDec: ID '(' ParamList ')' { $$ = mknode(FUNC_DEC, $3, NULL, NULL, yylineno); strcpy($$->type_id, $1); }
    | ID '(' ')' { $$ = mknode(FUNC_DEC, NULL, NULL, NULL, yylineno); strcpy($$->type_id, $1); } //函数名存放在$$->type_id
;
FuncDecStmt: Specifier FuncDec ';' { $$ = $2; $$->ptr[1] = $1;}
;
Stmt: Exp ';'    { $$=mknode(EXP_STMT,$1,NULL,NULL,yylineno); }
    | BlockStmt      {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
    | RETURN Exp ';'   {$$=mknode(RETURN,$2,NULL,NULL,yylineno);}
    | IF '(' Exp ')' Stmt %prec LOWER_THEN_ELSE   {$$=mknode(IF_THEN,$3,$5,NULL,yylineno);}
    | IF '(' Exp ')' Stmt ELSE Stmt   {$$=mknode(IF_THEN_ELSE,$3,$5,$7,yylineno);}
    | WHILE '(' Exp ')' Stmt {$$=mknode(WHILE,$3,$5,NULL,yylineno);}
;
BlockInnerStmtList: { $$=NULL;}
    | VarDecStmt BlockInnerStmtList {$$=mknode(COMP_LIST,$1,$2,NULL,yylineno);}
    | Stmt BlockInnerStmtList {$$=mknode(COMP_LIST,$1,$2,NULL,yylineno);}
;
BlockStmt: '{' BlockInnerStmtList '}' {$$=mknode(COMP_STM,$2,NULL,NULL,yylineno);}
;
FuncDef: Specifier FuncDec BlockStmt { $2->ptr[1] = $1; $$ = mknode(FUNC_DEF, $2, $3, NULL, yylineno); }
;
Exp: Var ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"'='");}//$$结点type_id空置未用，正好存放运算符
    | Var INC { $$=mknode(INC, $1, NULL, NULL, yylineno); strcpy($$->type_id, "post ++"); }
    | INC Var { $$=mknode(INC, $2, NULL, NULL, yylineno); strcpy($$->type_id, "pre ++"); }
    | Var DEC { $$=mknode(DEC, $1, NULL, NULL, yylineno); strcpy($$->type_id, "post --"); }
    | DEC Var { $$=mknode(DEC, $2, NULL, NULL, yylineno); strcpy($$->type_id, "pre --"); }
    | Exp AND Exp   {$$=mknode(AND,$1,$3,NULL,yylineno);strcpy($$->type_id,"AND");}
    | Exp OR Exp    {$$=mknode(OR,$1,$3,NULL,yylineno);strcpy($$->type_id,"OR");}
    | Exp CMP Exp {$$=mknode(CMP,$1,$3,NULL,yylineno);strcpy($$->type_id,$2);}  //词法分析关系运算符号自身值保存在$2中
    | Exp PLUS Exp  {$$=mknode(PLUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"'+'");}
    | Exp MINUS Exp {$$=mknode(MINUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"'-'");}
    | Exp STAR Exp  {$$=mknode(STAR,$1,$3,NULL,yylineno);strcpy($$->type_id,"'*'");}
    | Exp DIV Exp   {$$=mknode(DIV,$1,$3,NULL,yylineno);strcpy($$->type_id,"'/'");}
    | '(' Exp ')'     {$$=$2;}
    | MINUS Exp %prec UMINUS   {$$=mknode(UMINUS,$2,NULL,NULL,yylineno);strcpy($$->type_id,"UMINUS");}
    | NOT Exp       {$$=mknode(NOT,$2,NULL,NULL,yylineno);strcpy($$->type_id,"NOT");}
    | ID '(' ArgList ')' {$$=mknode(FUNC_CALL,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
    | ID '(' ')'      {$$=mknode(FUNC_CALL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
    | Var {$$ = $1;}
    | INT           {$$=mknode(INT,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT;}
    | FLOAT         {$$=mknode(FLOAT,NULL,NULL,NULL,yylineno);$$->type_float=$1;$$->type=FLOAT;}
    | CHAR         {$$=mknode(CHAR,NULL,NULL,NULL,yylineno);$$->type_char=$1;$$->type=CHAR;} 
;
ArgList: Exp ',' ArgList {$$=mknode(ARGS,$1,$3,NULL,yylineno);}
    | Exp {$$=mknode(ARGS,$1,NULL,NULL,yylineno);}
;
%%

int main(int argc, char *argv[]){
    yyin=fopen(argv[1],"r");
    if (!yyin) return 0;
    yylineno=1;
    yyparse();
    return 0;
    }

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}