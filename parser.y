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
%type  <ptr> program ExtDefList ExtDef  Specifier ExtDecList FuncDec CompSt VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args

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

program: ExtDefList    { display($1,0); /*semantic_Analysis0($1);*/}     /*显示语法树,语义分析*/
         ; 
ExtDefList: {$$=NULL;}
          | ExtDef ExtDefList {$$=mknode(EXT_DEF_LIST,$1,$2,NULL,yylineno);}   //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数
          ;  
ExtDef:   Specifier ExtDecList ';'   {$$=mknode(EXT_VAR_DEF,$1,$2,NULL,yylineno);}   //该结点对应一个外部变量声明
         |Specifier FuncDec CompSt    {$$=mknode(FUNC_DEF,$1,$2,$3,yylineno);}         //该结点对应一个函数定义
         | error ';'   {$$=NULL; }
         ;
Specifier:  TYPE    {$$=mknode(TYPE,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->type=resolve_type($1);}   
           ;      
ExtDecList:  VarDec      {$$=$1;}       /*每一个EXT_DECLIST的结点，其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
           | VarDec ',' ExtDecList {$$=mknode(EXT_DEC_LIST,$1,$3,NULL,yylineno);}
           ;  
VarDec:  ID          {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
         ;
FuncDec: ID '(' VarList ')'   {$$=mknode(FUNC_DEC,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
        |ID '('  ')'   {$$=mknode(FUNC_DEC,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id

        ;  
VarList: ParamDec  {$$=mknode(PARAM_LIST,$1,NULL,NULL,yylineno);}
        | ParamDec ','  VarList  {$$=mknode(PARAM_LIST,$1,$3,NULL,yylineno);}
        ;
ParamDec: Specifier VarDec         {$$=mknode(PARAM_DEC,$1,$2,NULL,yylineno);}
         ;

CompSt: '{' DefList StmList '}'    {$$=mknode(COMP_STM,$2,$3,NULL,yylineno);}
       ;
StmList: {$$=NULL; }  
        | Stmt StmList  {$$=mknode(STM_LIST,$1,$2,NULL,yylineno);}
        ;
Stmt:   Exp ';'    {$$=mknode(EXP_STMT,$1,NULL,NULL,yylineno);}
      | CompSt      {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
      | RETURN Exp ';'   {$$=mknode(RETURN,$2,NULL,NULL,yylineno);}
      | IF '(' Exp ')' Stmt %prec LOWER_THEN_ELSE   {$$=mknode(IF_THEN,$3,$5,NULL,yylineno);}
      | IF '(' Exp ')' Stmt ELSE Stmt   {$$=mknode(IF_THEN_ELSE,$3,$5,$7,yylineno);}
      | WHILE '(' Exp ')' Stmt {$$=mknode(WHILE,$3,$5,NULL,yylineno);}
      ;
  
DefList: {$$=NULL; }
        | Def DefList {$$=mknode(DEF_LIST,$1,$2,NULL,yylineno);}
        ;
Def:    Specifier DecList ';' {$$=mknode(VAR_DEF,$1,$2,NULL,yylineno);}
        ;
DecList: Dec  {$$=mknode(DEC_LIST,$1,NULL,NULL,yylineno);}
       | Dec ',' DecList  {$$=mknode(DEC_LIST,$1,$3,NULL,yylineno);}
       ;
Dec:     VarDec  {$$=$1;}
       | VarDec ASSIGNOP Exp  {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"'='");}
       ;
Exp:   VarDec ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"'='");}//$$结点type_id空置未用，正好存放运算符
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
      | ID '(' Args ')' {$$=mknode(FUNC_CALL,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID '(' ')'      {$$=mknode(FUNC_CALL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID            {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | INT           {$$=mknode(INT,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT;}
      | FLOAT         {$$=mknode(FLOAT,NULL,NULL,NULL,yylineno);$$->type_float=$1;$$->type=FLOAT;}
      | CHAR         {$$=mknode(CHAR,NULL,NULL,NULL,yylineno);$$->type_char=$1;$$->type=CHAR;} 
      ;
Args:    Exp ',' Args    {$$=mknode(ARGS,$1,$3,NULL,yylineno);}
       | Exp               {$$=mknode(ARGS,$1,NULL,NULL,yylineno);}
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