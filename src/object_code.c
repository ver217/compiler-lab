#include "../include/def.h"
#include "../include/util.h"

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

void assign(FILE* fp, struct codenode* h, char* base_addr_reg) {
    if (h->opn1.kind == ID) {
        fprintf(fp, "  lw $t1, %d(%s)\n", h->opn1.offset, base_addr_reg);
        fprintf(fp, "  move $t3, $t1\n");
        fprintf(fp, "  sw $t3, %d(%s)\n", h->result.offset, base_addr_reg);
    } else {
        if (h->opn1.kind == FLOAT)
            fprintf(fp, "  li $t3, %f\n", h->opn1.const_float);
        else if (h->opn1.kind == INT)
            fprintf(fp, "  li $t3, %d\n", h->opn1.const_int);
        else
            fprintf(fp, "  li $t3, %d\n", h->opn1.const_char);
        int width = resolve_size(h->result.type);
        fprintf(fp, "  s%c $t3, %d(%s)\n", get_size_flag(width), h->result.offset, base_addr_reg);
    }
}

void branch_prefix(FILE* fp, struct codenode* h) {
    fprintf(fp, "  l%c $t1, %d($sp)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset);
    fprintf(fp, "  l%c $t2, %d($sp)\n", get_size_flag(resolve_size(h->opn2.type)), h->opn2.offset);
}

void bin_op(FILE* fp, struct codenode* h, char* instruction) {
    fprintf(fp, "  l%c $t1, %d($sp)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset);
    fprintf(fp, "  l%c $t2, %d($sp)\n", get_size_flag(resolve_size(h->opn2.type)), h->opn2.offset);
    fprintf(fp, "  %s\n", instruction);
    fprintf(fp, "  s%c $t3, %d($sp)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset);
}

void single_op(FILE* fp, struct codenode* h, char* instruction) {
    fprintf(fp, "  l%c $t1, %d($sp)\n", get_size_flag(resolve_size(h->opn1.type)), h->opn1.offset);
    fprintf(fp, "  %s\n", instruction);
    fprintf(fp, "  s%c $t3, %d($sp)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset);
}

void insert_stack(FILE* fp, char* reg, int offset) {
    fprintf(fp, "  addi $sp, $sp, -4\n");
    fprintf(fp, "  sw %s, -%d($sp)\n", reg, offset);
}

void objectCode(struct codenode *head, symbol_t* symbol_table, FILE* fp) {
    struct codenode *h = head;
    // global variables init
    fprintf(fp, "# global variabls init\n");
    for (symbol_t* symbol = symbol_table; symbol != NULL; symbol = symbol->hh.next) {
        if (symbol->level == 0 && symbol->flag == 'V') {
            int width = resolve_size(symbol->type);
            if (width == 1) {
                fprintf(fp, "  sb $0, %d($0)\n", symbol->offset); // 静态数据区起始地址为0
            } else
                fprintf(fp, "  sw $0, %d($0)\n", symbol->offset);
        }
    }
    do {
        if (h->result.level == 0 && h->op == ASSIGNOP)      // 全局变量初始化
            assign(fp, h, "$0");
        else
            break;
        h = h->next;
    } while (h != head);
    // main wrap
    fprintf(fp, "# main function wrapper\n");
    fprintf(fp, "  jal main\n");
    fprintf(fp, "  move $a0, $v0\n");
    fprintf(fp, "  li $v0, 17\n");
    fprintf(fp, "  syscall\n");
    // code
    fprintf(fp, "# code\n");
    do {
        switch (h->op) {
        case ASSIGNOP:
            assign(fp, h, "$sp");
            break;
        case PLUS:
            bin_op(fp, h, "add $t3, $t1, $t2");
            break;
        case MINUS:
            bin_op(fp, h, "sub $t3, $t1, $t2");
            break;
        case STAR:
            bin_op(fp, h, "mul $t3, $t1, $t2");
            break;
        case DIV:
            bin_op(fp, h, "mul $t3, $t1, $t2\ndiv $t1, $t2\nmflo $t3");
            break;
        case FUNCTION:
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
            // fprintf(fp, "l%c $t3, %d($sp)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset);
            // insert_stack(fp, "$t3", );
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
            fprintf(fp, "  l%c $v0, %d($sp)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset);
            fprintf(fp, "  jr $ra\n");
            break;
        case CALL: {
            fprintf(fp, "  move $s0, $sp\n");
            fprintf(fp, "  addi $sp, $sp, -%d\n", h->opn1.func_symbol->paramnum * 4 + h->opn1.func_symbol->offset); // 为形参和局部变量开辟栈空间
            int i;
            list_t* l;
            for (l = param_list, i = 0; i < h->opn1.func_symbol->paramnum; i++, l = l->next) {
                fprintf(fp, "  l%c $t3, %d($s0)\n", get_size_flag(resolve_size(l->h->result.type)), l->h->result.offset);
                fprintf(fp, "  sw $t3, %d($sp)\n", i * 4);
            }
            fprintf(fp, "  jal %s\n", h->opn1.id);
            fprintf(fp, "  addi $sp, $sp, %d\n", h->opn1.func_symbol->paramnum * 4 + h->opn1.func_symbol->offset);
            delete_param_list(param_list);
            param_list = NULL;
            if (strlen(h->result.id) > 0) {
                fprintf(fp, "  move $t3, $v0\n");
                fprintf(fp, "  s%c $t3, %d($sp)\n", get_size_flag(resolve_size(h->result.type)), h->result.offset);
            }
            break;
        }
        default:
            break;
        }
        h = h->next;
    } while (h != head);
}