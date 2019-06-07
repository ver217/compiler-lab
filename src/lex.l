%option yylineno
/*%option debug */
%{
#include <string.h>
#include "../include/def.h"
#include "../output/parser.tab.h"
#include "../include/util.h"

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

{int} { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, INT); yylval.type_int=atoi(yytext); return INT;}
{float} { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, FLOAT); yylval.type_float=atof(yytext); return FLOAT;}

"int" |
"float" |
"char" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, TYPE); strcpy(yylval.type_id, yytext); return TYPE;}
"void" { return VOID; }
"return" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, RETURN); return RETURN;}
"if" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, IF); return IF;}
"else" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, ELSE); return ELSE;}
"while" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, WHILE); return WHILE;}
{id} { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, ID); strcpy(yylval.type_id,  yytext); return ID;}
"/=" |
"*=" |
"+=" |
"-=" |
"%=" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, COMPASSIGN); strcpy(yylval.type_id, yytext); return COMPASSIGN; }
"=" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, ASSIGNOP); return ASSIGNOP;}
"+" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, PLUS); return PLUS;}
"-" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, MINUS); return MINUS;}
"*" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, STAR); return STAR;}
"/" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, DIV); return DIV;}
"," |
";" |
"(" |
")" |
"{" |
"}" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, -1); return yytext[0];}
\n {yycolumn = 1;}
"&&" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, AND); return AND;}
"||" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, OR); return OR;}
"!" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, NOT); return NOT;}
"++" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, INC); return INC;}
"--" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, DEC); return DEC;}
">" |
"<" |
"==" |
">=" |
"<=" |
"!=" { printf("%d: token:`%s`, type enum:%d\n", yylineno, yytext, CMP); strcpy(yylval.type_id,  yytext); return CMP;}
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

