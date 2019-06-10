#include "../include/def.h"
#include "../include/util.h"

void objectCode(struct codenode *head, symbol_t* symbol_table, FILE* fp) {
    // global variables init
    for (symbol_t* symbol = symbol_table; symbol != NULL; symbol = symbol->hh.next) {
        if (symbol->level == 0 && symbol->flag == 'V') {
            int width = resolve_size(symbol->type);
            if (width == 1) {
                fprintf(fp, "sb $zero, %d($zero)", symbol->offset); // 静态数据区起始地址为0
            } else
                fprintf(fp, "sw $zero, %d($zero)", symbol->offset);
        }
    }
}