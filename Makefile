.PHONY: all clean compress

ast: src/lex.l src/parser.y src/ast.c include/util.h src/util.c src/semantic.c src/object_code.c
	bison -d -v src/parser.y
	flex src/lex.l
	mv parser.* lex.yy.c output
	gcc -g output/parser.tab.c output/lex.yy.c src/ast.c src/util.c src/semantic.c src/object_code.c -o bin/ast

clean:
	rm -f output/* bin/*

compress: src/ast.c include/def.h src/lex.l src/parser.y include/util.h src/util.c src/semantic.c src/object_code.c Makefile
	zip source.zip src/ast.c include/def.h src/lex.l src/parser.y include/util.h src/util.c src/semantic.c src/object_code.c Makefile