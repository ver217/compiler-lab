#pragma once
#include "../output/parser.tab.h"

int resolve_type(char* str);

char atoc(char* str);

int htod(char* str);

int otod(char* str);

int str_to_op(char* str, int flag);

int resolve_aluop(char op);