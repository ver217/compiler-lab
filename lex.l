%option yylineno
/*%option debug */
%{
#include <string.h>
#include "def.h"
#include "parser.tab.h"

int yycolumn = 1;
#define YY_USER_ACTION    	yylloc.first_line=yylloc.last_line=yylineno; \
	yylloc.first_column=yycolumn;	yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;
typedef union {
    int type_int;
	float type_float;
	char type_char;
    char type_id[32];
	struct node *ptr;
} YYLVAL;
#define YYSTYPE YYLVAL

%}
id   [A-Za-z][A-Za-z0-9]*
int    [0-9]+
float ([0-9]*\.[0-9]+)|([0-9]+\.)
char \'([^'\\]|\\['"?\\abfnrtv])+\'
%%
{int} {yylval.type_int=atoi(yytext); return INT;}
{float} {yylval.type_float=atof(yytext); return FLOAT;}
{char} { yylval.type_char=yytext[1]; return CHAR; }
"int" |
"float" |
"char" {strcpy(yylval.type_id, yytext); return TYPE;}
"return" {return RETURN;}
"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
{id} {strcpy(yylval.type_id,  yytext); return ID;}
"=" {return ASSIGNOP;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return STAR;}
"/" {return DIV;}
"," |
";" |
"(" |
")" |
"{" |
"}" {return yytext[0];}
\n {yycolumn = 1;}
"&&" {return AND;}
"||" {return OR;}
"!" {return NOT;}
"++" {return INC;}
"--" {return DEC;}
">" |
"<" |
"==" |
">=" |
"<=" |
"!=" {strcpy(yylval.type_id,  yytext); return CMP;}
[ \t\r]+
"//".*\n
. {printf("error in line %d\n",yylineno);}
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
    return 1;
}

