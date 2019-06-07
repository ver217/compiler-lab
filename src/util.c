#include "../include/util.h"
#include "../include/def.h"
#include <string.h>
#include <stdlib.h>

int resolve_type(char* str) {
    if (strcmp(str, "int") == 0)
        return INT;
    else if (strcmp(str, "float") == 0)
        return FLOAT;
    else if (strcmp(str, "char") == 0)
        return CHAR;
    else
        return VOID;
}

// hex to decimal
int htod(char* str) {
    int val = 0;
    while (*str != '\0') {
        val *= 16;
        if ('0' <= *str && *str <= '9')
            val += *str - '0';
        else if ('a' <= *str && *str <= 'f')
            val += *str - 'a';
        else if ('A' <= *str && *str <= 'F')
            val += *str - 'A';
        else
            return 0;
        str++;
    }
    return val;
}

// oct to decimal
int otod(char* str) {
    int val = 0;
    while (*str != '\0') {
        val *= 8;
        if ('0' <= *str && *str <= '7')
            val += *str - '0';
        else
            return 0;
        str++;
    }
    return val;
}

char atoc(char* str) {
    size_t len = strlen(str);
    if (len == 1)
        return str[0];
    if (str[0] != '\\')
        return '\0';
    if (len > 4)
        return '\0';
    if (len == 2) {
        switch (str[1]) {
        case 'a':
        case 'b':
        case 'f':
            return str[1] - 90;
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '\\':
        case '?':
        case '\'':
        case '"':
            return str[1];
        case '0':
            return '\0';
        }
    } else {
        char c = 0;
        if (str[1] == 'x')
            return (char)htod(str + 2);
        else
            return (char)otod(str + 2);
    }
    return '\0';
}

int str_to_op(char* str, int flag) {
    if (flag == 0) {
        if (strcmp(str, "<") == 0)
            return JL;
        else if (strcmp(str, "<=") == 0)
            return JLE;
        else if (strcmp(str, ">") == 0)
            return JG;
        else if (strcmp(str, ">=") == 0)
            return JGE;
        else if (strcmp(str, "==") == 0)
            return JE;
        else if (strcmp(str, "!=") == 0)
            return JNE;
    } else {
        if (strcmp(str, "<") == 0)
            return JLF;
        else if (strcmp(str, "<=") == 0)
            return JLEF;
        else if (strcmp(str, ">") == 0)
            return JGF;
        else if (strcmp(str, ">=") == 0)
            return JGEF;
        else if (strcmp(str, "==") == 0)
            return JEF;
        else if (strcmp(str, "!=") == 0)
            return JNEF;
    }
    return -1;
}