#include "util.h"
#include <string.h>

int resolve_type(char* str) {
    if (strcmp(str, "int") == 0)
        return INT;
    else if (strcmp(str, "float") == 0)
        return FLOAT;
    else
        return CHAR;
}