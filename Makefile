.PHONY: all clean

all: lex.l parser.y ast.c util.h util.c
	bison -d -v parser.y
	flex lex.l
	gcc -g parser.tab.c lex.yy.c ast.c util.c

clean:
	rm -f parser.tab.c parser.tab.h parser.output lex.yy.c a.out