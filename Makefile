.PHONY: all clean compress

all: lex.l parser.y ast.c util.h util.c
	bison -d -v parser.y
	flex lex.l
	gcc -g parser.tab.c lex.yy.c ast.c util.c

clean:
	rm -f parser.tab.c parser.tab.h parser.output lex.yy.c a.out

compress: ast.c def.h lex.l parser.y util.h util.c Makefile
	zip source.zip ast.c def.h lex.l parser.y util.h util.c Makefile