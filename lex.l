%option yylineno
/*%option debug */
%{
#include <string.h>
#include "def.h"
#include "parser.tab.h"
#include "util.h"

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

%x COMMENT
%x CHARSEQ

id   [A-Za-z][A-Za-z0-9]*
int    [0-9]+
float ([0-9]*\.[0-9]+)|([0-9]+\.)
char \'([^'\\]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[xX][0-9a-fA-F]+)+\'
string \"([^"\\]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[xX][0-9a-fA-F]+)*\"
inner_char ([^'\\\n]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[xX][0-9a-fA-F]+)+
inner_str ([^"\\\n]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[xX][0-9a-fA-F]+)*
%%

"//".*\n
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>> { printf("unterminated comment in line %d\n",yylineno); return 0; }
"'" { BEGIN(CHARSEQ); }
<CHARSEQ>"'" { BEGIN(INITIAL); }
<CHARSEQ>\n |
<CHARSEQ><<EOF>> { printf("unterminated char seq in line %d\n",yylineno); return 0; }
<CHARSEQ>{inner_char}\' { unput('\''); yytext[yyleng-1] = '\0'; yylval.type_char = atoc(yytext); return CHAR;}
<CHARSEQ>.

{int} {yylval.type_int=atoi(yytext); return INT;}
{float} {yylval.type_float=atof(yytext); return FLOAT;}

"int" |
"float" |
"char" {strcpy(yylval.type_id, yytext); return TYPE;}
"return" {return RETURN;}
"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
{id} {strcpy(yylval.type_id,  yytext); return ID;}
"/=" |
"*=" |
"+=" |
"-=" |
"%=" { strcpy(yylval.type_id, yytext); return COMPASSIGN; }
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

