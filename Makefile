.PHONY: all clean

all: lex.l exp.y display.c
	bison -d -v exp.y
	flex lex.l
	gcc exp.tab.c lex.yy.c display.c -Lfl -Ly

clean:
	rm -f exp.tab.c exp.tab.h exp.output lex.yy.c a.out