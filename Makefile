.PHONY: all clean

all: lex.l parser.y ast.c
	bison -d -v parser.y
	flex lex.l
	gcc parser.tab.c lex.yy.c ast.c -Lfl -Ly

clean:
	rm -f parser.tab.c parser.tab.h parser.output lex.yy.c a.out