#pragma once
#include "../output/parser.tab.h"

#define typeof(TYPE) \
(TYPE == INT ? "int" : TYPE == FLOAT ? "float" : TYPE == CHAR ? "char" : TYPE == VOID ? "void" : "null")

int resolve_type(char* str);

char atoc(char* str);

int htod(char* str);

int otod(char* str);

int str_to_op(char* str, int flag);

int resolve_aluop(char op);

int resolve_size(int TYPE);

long current_timestamp();

char* ltoa(long num, char* str);