#include "../include/def.h"
#include "../include/util.h"

#define ADDRBASE(level) (level == 0 ? "$s0" : "$sp")

typedef struct list {
    struct codenode* h;
    struct list* next;
} list_t;

list_t* new_param_list(struct codenode* h) {
    list_t* l = (list_t*)malloc(sizeof(list_t));
    l->h = h;
    l->next = NULL;
    return l;
}

void delete_param_list(list_t* l) {
    if (l) {
        delete_param_list(l->next);
        free(l);
    }
}

void insert_param_list(list_t* l, struct codenode* h) {
    if (!l)
        return;
    while (l->next)
        l = l->next;
    l->next = new_param_list(h);
}

list_t* param_list = NULL;

char get_size_flag(int size) {
    switch (size) {
    case 4:
        return 'w';
    case 2:
        return 'h';
    case 1:
        return 'b';
    }
    return '\0';
}

void assign(FILE* fp, struct codenode* h) {
    if (h->opn1.kind == ID) {
        fprintf(fp, "  lw $t1, %d(%s)\n", h->opn1.offset, ADDRBASE(h->opn1.level));
        fprintf(fp, "  move $t3, $t1\n");
        fprintf(fp, "  sw $t3, %d(%s)\n", h->result.offset, ADDRBASE(h->result.level));
    } else {
        if (h->opn1.kind == FLOAT)
            fprintf(fp, "  li $t3, %f\n", h->opn1.const_float);
        else if (h->opn1.kind == INT)
            fprintf(fp, "  li $t3, %d\n", h->opn1.const_int);
        else
            fprintf(fp, "  li $t3, %d\n", h->opn1.const_char);
        int width = resolve_size(h->result.type);
        fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(width), h->result.offset, ADDRBASE(h->result.level));
    }
}

void branch_prefix(FILE* fp, struct codenode* h) {
    fprintf(fp, "  l%c $t1, %d(%s)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset, ADDRBASE(h->opn1.level));
    fprintf(fp, "  l%c $t2, %d(%s)\n", get_size_flag(resolve_size(h->opn2.type)), h->opn2.offset, ADDRBASE(h->opn2.level));
}

void bin_op(FILE* fp, struct codenode* h, char* instruction) {
    fprintf(fp, "  l%c $t1, %d(%s)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset, ADDRBASE(h->opn1.level));
    fprintf(fp, "  l%c $t2, %d(%s)\n", get_size_flag(resolve_size(h->opn2.type)), h->opn2.offset, ADDRBASE(h->opn2.level));
    fprintf(fp, "  %s\n", instruction);
    fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset, ADDRBASE(h->result.level));
}

void bin_inst_op(FILE* fp, struct codenode* h, char* instruction) {
    fprintf(fp, "  l%c $t1, %d(%s)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset, ADDRBASE(h->opn1.level));
    fprintf(fp, "  %s $t3, $t1, %d\n", instruction, h->opn2.const_int);
    fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset, ADDRBASE(h->result.level));
}

void single_op(FILE* fp, struct codenode* h, char* instruction) {
    fprintf(fp, "  l%c $t1, %d(%s)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset, ADDRBASE(h->opn1.level));
    fprintf(fp, "  %s\n", instruction);
    fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset, ADDRBASE(h->result.level));
}

void objectCode(struct codenode *head, symbol_t* symbol_table, FILE* fp) {
    struct codenode *h = head;
    fprintf(fp, "main:\n");
    fprintf(fp, "  li $s0, 0\n");
    // global variables init
    fprintf(fp, "# global variables init\n");
    for (symbol_t* symbol = symbol_table; symbol != NULL; symbol = symbol->hh.next) {
        if (symbol->level == 0 && symbol->flag == 'V') {
            int width = resolve_size(symbol->type);
            if (width == 1) {
                fprintf(fp, "  sb $0, %d($s0)\n", symbol->offset); // 静态数据区起始地址为$s0
            } else
                fprintf(fp, "  sw $0, %d($s0)\n", symbol->offset);
        }
    }
    do {
        if (h->result.level == 0 && h->op == ASSIGNOP)      // 全局变量初始化
            assign(fp, h);
        else
            break;
        h = h->next;
    } while (h != head);
    // main wrap
    fprintf(fp, "# main function wrapper\n");
    symbol_t* main_func;
    HASH_FIND_STR(symbol_table, "main", main_func);
    if (main_func == NULL) {
        printf("need main function!\n");
        exit(-1);
    }
    fprintf(fp, "  addi $sp, $sp, -%d\n", main_func->paramnum * 4 + main_func->offset);
    fprintf(fp, "  jal _main\n");
    fprintf(fp, "  addi $sp, $sp, %d\n", main_func->paramnum * 4 + main_func->offset);
    fprintf(fp, "  move $a0, $v0\n");
    fprintf(fp, "  li $v0, 17\n");
    fprintf(fp, "  syscall\n");
    // code
    fprintf(fp, "# code\n");
    do {
        switch (h->op) {
        case ASSIGNOP:
            assign(fp, h);
            break;
        case PLUS:
            if (h->opn2.kind == ID)
                bin_op(fp, h, "add $t3, $t1, $t2");
            else
                bin_inst_op(fp, h, "addi");
            break;
        case MINUS:
            if (h->opn2.kind == ID)
                bin_op(fp, h, "sub $t3, $t1, $t2");
            else
                bin_inst_op(fp, h, "subi");
            break;
        case STAR:
            bin_op(fp, h, "mul $t3, $t1, $t2");
            break;
        case DIV:
            bin_op(fp, h, "mul $t3, $t1, $t2\ndiv $t1, $t2\nmflo $t3");
            break;
        case FUNCTION:
            fprintf(fp, "_%s:\n", h->result.id);
            break;
        case LABEL:
            fprintf(fp, "%s:\n", h->result.id);
            break;
        case GOTO:
            fprintf(fp, "  j %s\n", h->result.id);
            break;
        case JLE:
            branch_prefix(fp, h);
            fprintf(fp, "  ble $t1, $t2, %s\n", h->result.id);
            break;
        case JL:
            branch_prefix(fp, h);
            fprintf(fp, "  blt $t1, $t2, %s\n", h->result.id);
            break;
        case JGE:
            branch_prefix(fp, h);
            fprintf(fp, "  bge $t1, $t2, %s\n", h->result.id);
            break;
        case JG:
            branch_prefix(fp, h);
            fprintf(fp, "  bgt $t1, $t2, %s\n", h->result.id);
            break;
        case JE:
            branch_prefix(fp, h);
            fprintf(fp, "  beq $t1, $t2, %s\n", h->result.id);
            break;
        case JNE:
            branch_prefix(fp, h);
            fprintf(fp, "  bne $t1, $t2, %s\n", h->result.id);
            break;
        case JLEF:
            branch_prefix(fp, h);
            fprintf(fp, "  bgt $t1, $t2, %s\n", h->result.id);
            break;
        case JLF:
            branch_prefix(fp, h);
            fprintf(fp, "  bge $t1, $t2, %s\n", h->result.id);
            break;
        case JGEF:
            branch_prefix(fp, h);
            fprintf(fp, "  blt $t1, $t2, %s\n", h->result.id);
            break;
        case JGF:
            branch_prefix(fp, h);
            fprintf(fp, "  ble $t1, $t2, %s\n", h->result.id);
            break;
        case JEF:
            branch_prefix(fp, h);
            fprintf(fp, "  bne $t1, $t2, %s\n", h->result.id);
            break;
        case JNEF:
            branch_prefix(fp, h);
            fprintf(fp, "  beq $t1, $t2, %s\n", h->result.id);
            break;
        case ARG:
            if (param_list == NULL)
                param_list = new_param_list(h);
            else
                insert_param_list(param_list, h);
            break;
        case NOT:
            single_op(fp, h, "nor $t3, $t1, $t1");
            break;
        case UMINUS:
            single_op(fp, h, "sub $t3, $0, $t1");
            break;
        case AND:
            bin_op(fp, h, "and $t3, $t1, $t2");
            break;
        case OR:
            bin_op(fp, h, "or $t3, $t1, $t2");
            break;
        case CMP:
            switch (h->result.cmp_type) {
            case JLE:
                bin_op(fp, h, "sle $t3, $t1, $t2");
                break;
            case JL:
                bin_op(fp, h, "slt $t3, $t1, $t2");
                break;
            case JGE:
                bin_op(fp, h, "sge $t3, $t1, $t2");
                break;
            case JG:
                bin_op(fp, h, "sgt $t3, $t1, $t2");
                break;
            case JE:
                bin_op(fp, h, "seq $t3, $t1, $t2");
                break;
            case JNE:
                bin_op(fp, h, "sne $t3, $t1, $t2");
                break;
            }
        case RETURN:
            fprintf(fp, "  l%c $v0, %d(%s)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset, ADDRBASE(h->result.level));
            fprintf(fp, "  jr $ra\n");
            break;
        case CALL: {
            fprintf(fp, "  move $s1, $sp\n");
            fprintf(fp, "  addi $sp, $sp, -%d\n", (h->opn1.func_symbol->paramnum + 1) * 4 + h->opn1.func_symbol->offset); // 为形参和局部变量开辟栈空间
            int i;
            list_t* l;
            for (l = param_list, i = 1; i < h->opn1.func_symbol->paramnum + 1; i++, l = l->next) {
                fprintf(fp, "  l%c $t3, %d($s1)\n", get_size_flag(resolve_size(l->h->result.type)), l->h->result.offset);
                fprintf(fp, "  sw $t3, %d($sp)\n", i * 4);
            }
            fprintf(fp, "  sw $ra, 0($sp)\n");
            fprintf(fp, "  jal _%s\n", h->opn1.id);
            fprintf(fp, "  lw $ra, 0($sp)\n");
            fprintf(fp, "  addi $sp, $sp, %d\n", (h->opn1.func_symbol->paramnum + 1) * 4 + h->opn1.func_symbol->offset);
            delete_param_list(param_list);
            param_list = NULL;
            if (strlen(h->result.id) > 0) {
                fprintf(fp, "  move $t3, $v0\n");
                fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset, ADDRBASE(h->result.level));
            }
            break;
        }
        default:
            break;
        }
        h = h->next;
    } while (h != head);
}