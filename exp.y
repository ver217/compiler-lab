%{
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Node.h"
extern char *yytext;
extern FILE *yyin;
void display(struct Exp *,int);
%}

%union {
    int type_int;
    char type_id[32];
    float type_float;
    char type_char;
    struct Exp *pExp;
};

%type  <pExp> line exp       /*指定line和exp的语义值是结点指针pExp*/
%token <type_int> INT        /*指定INT的语义值是type_int，有词法分析得到的数值*/
%token <type_id> ID TYPE CMP          /*指定ID的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_float> FLOAT
%token <type_char> CHAR

%token AND OR NOT RETURN IF ELSE WHILE/*用bison对该文件编译时，带参数-d，生成的exp.tab.h中
                                      给这些单词进行编码，可在lex.l中包含exp.tab.h使用这些单词种类码*/

%right '='
%left OR
%left AND
%left CMP
%left '+' '-'
%left '*' '/'
%right UMINUS NOT

%nonassoc ELSE
%%
input:
    | input line
     ;
line: '\n'    { ;}
    | exp '\n' { display($1,0);}                  /*显示语法树*/
    | error '\n' { printf("exp error!\n");}       /*一旦有语法错误，跳过这行*/
    ;
exp: INT {$$=new_node(INT_NODE, NULL, NULL); $$->type_int=$1;}
   | TYPE ID {$$=new_node(ID_NODE, NULL, NULL); strcpy($$->type_id,$2);}
   | exp '+' exp {$$=new_node(PLUS_NODE, $1, $3);}
   | exp '-' exp {$$=new_node(MINUS_NODE, $1, $3);}
   | exp '*' exp {$$=new_node(STAR_NODE, $1, $3);}
   | exp '/' exp {$$=new_node(DIV_NODE, $1, $3);}
   | '(' exp ')' {$$=(PEXP)$2;}
   | '-' exp %prec UMINUS {$$=new_node(UMINUS_NODE, $2, NULL);}  
   ;
    /*以上exp的规则的语义动作生成抽象语法树*/
%%

int main(int argc, char *argv[]){
    yyin=fopen(argv[1],"r");
    if (!yyin) return;
    yyparse();
    return 0;
    }
    
yyerror(char *s){
   printf("%s   %s \n",s,yytext);
   
 }

/*
命令序列：
flex lex.l
bison -d -v exp.y
gcc -o exp  exp.tab.c lex.yy.c display.c -Lfl -Ly
*/

