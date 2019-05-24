%option yylineno
%{
#include "exp.tab.h"
#include "string.h"
#include "Node.h"

typedef union {
    int type_int;
	float type_float;
	char type_char;
    char type_id[32];
    PEXP *pexp;
} YYLVAL;
#define YYSTYPE YYLVAL

%}
id   [A-Za-z][A-Za-z0-9]*
int    [0-9]+
float ([0-9]*\.[0-9]+)|([0-9]+\.)
char '(.|\n)'
%%
{int}      {yylval.type_int=atoi(yytext); return INT;}
{float}    {yylval.type_float=atof(yytext); return FLOAT;}
{char}     {yylval.type_char=yytext[1]; return CHAR;}
"int"      {strcpy(yylval.type_id, yytext); return TYPE;}
"float"      {strcpy(yylval.type_id, yytext); return TYPE;}
"char"      {strcpy(yylval.type_id, yytext); return TYPE;}
"return"   {return RETURN;}
"if"       {return IF;}
"else"     {return ELSE;}
"while"    {return WHILE;}
{id}       {strcpy(yylval.type_id,  yytext);
            return ID;}
[=]        {return ASSIGNOP;}
[+]        {return PLUS;}
[-]        {return MINUS;}
[*]        {return STAR;}
[/]        {return DIV;}
"&&"         {return AND;}
"||"       {return OR;}
"!"          {return NOT;}
[,]        {return COMMA;}
[(]        {return LP;}
[)]        {return RP;}
[{]        {return LC;}
[}]        {return RC;}
[;]        {return yytext[0];}
[ \t\r]      {;}
[\n]       {return yytext[0];}
.          {printf("error in line %d\n",yylineno);}
%%

/* 和bison联用时，不需要这部分
void main()
{
yylex();
return 0;
}

*/
int yywrap()
{
    yylex();
    return 0;
//return 1;
}

