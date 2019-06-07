#include "../include/def.h"
#include "../include/util.h"
#include "../include/uthash.h"

symbol_t* new_symbol(char* name, char* alias, int level, int type, int flag, int offset) {
    symbol_t* symbol = (symbol_t*)malloc(sizeof(symbol_t));
    strcpy(symbol->name, name);
    strcpy(symbol->alias, alias);
    symbol->level = level;
    symbol->type = type;
    symbol->flag = flag;
    symbol->offset = offset;
    return symbol;
}

char *strcat0(char *s1, char *s2) {
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *newAlias() {
    static int no = 1;
    char s[10];
    sprintf(s, "%d", no++);
    return strcat0("v", s);
}

char *newLabel() {
    static int no = 1;
    char s[10];
    sprintf(s, "%d", no++);
    return strcat0("L", s);
}

char *newTemp() {
    static int no = 1;
    char s[10];
    sprintf(s, "%d", no++);
    return strcat0("t", s);
}

//生成一条TAC代码的结点组成的双向循环链表，返回头指针
struct codenode *genIR(int op, struct opn opn1, struct opn opn2, struct opn result) {
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = op;
    h->opn1 = opn1;
    h->opn2 = opn2;
    h->result = result;
    h->next = h->prior = h;
    return h;
}

//生成一条标号语句，返回头指针
struct codenode *genLabel(char *label) {
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = LABEL;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

//生成GOTO语句，返回头指针
struct codenode *genGoto(char *label) {
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = GOTO;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

//合并多个中间代码的双向循环链表，首尾相连
struct codenode *merge(int num, ...) {
    struct codenode *h1, *h2, *p, *t1, *t2;
    va_list ap;
    va_start(ap, num);
    h1 = va_arg(ap, struct codenode *);
    while (--num > 0) {
        h2 = va_arg(ap, struct codenode *);
        if (h1 == NULL) h1 = h2;
        else if (h2) {
            t1 = h1->prior;
            t2 = h2->prior;
            t1->next = h2;
            t2->next = h1;
            h1->prior = t2;
            h2->prior = t1;
        }
    }
    va_end(ap);
    return h1;
}

//输出中间代码
void prnIR(struct codenode *head) {
    char opnstr1[32], opnstr2[32], resultstr[32];
    struct codenode *h = head;
    do {
        if (h->opn1.kind == INT)
            sprintf(opnstr1, "#%d", h->opn1.const_int);
        if (h->opn1.kind == FLOAT)
            sprintf(opnstr1, "#%f", h->opn1.const_float);
        if (h->opn1.kind == ID)
            sprintf(opnstr1, "%s", h->opn1.id);
        if (h->opn2.kind == INT)
            sprintf(opnstr2, "#%d", h->opn2.const_int);
        if (h->opn2.kind == FLOAT)
            sprintf(opnstr2, "#%f", h->opn2.const_float);
        if (h->opn2.kind == ID)
            sprintf(opnstr2, "%s", h->opn2.id);
        sprintf(resultstr, "%s", h->result.id);
        switch (h->op) {
        case ASSIGNOP:
            printf("  %s = %s\n", resultstr, opnstr1);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            printf("  %s = %s %c %s\n", resultstr, opnstr1, \
                   h->op == PLUS ? '+' : h->op == MINUS ? '-' : h->op == STAR ? '*' : '\\', opnstr2);
            break;
        case FUNCTION:
            printf("\nfunction %s :\n", h->result.id);
            break;
        case PARAM:
            printf("  param %s\n", h->result.id);
            break;
        case LABEL:
            printf("%s:\n", h->result.id);
            break;
        case GOTO:
            printf("  goto %s\n", h->result.id);
            break;
        case JLE:
            printf("  if %s <= %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JL:
            printf("  if %s < %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGE:
            printf("  if %s >= %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JG:
            printf("  if %s > %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JE:
            printf("  if %s == %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JNE:
            printf("  if %s != %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JLEF:
            printf("  ifFalse %s <= %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JLF:
            printf("  ifFalse %s < %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGEF:
            printf("  ifFalse %s >= %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGF:
            printf("  ifFalse %s > %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JEF:
            printf("  ifFalse %s == %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JNEF:
            printf("  ifFalse %s != %s goto %s\n", opnstr1, opnstr2, resultstr);
            break;
        case ARG:
            printf("  ARG %s\n", h->result.id);
            break;
        case CALL:
            printf("  %s = CALL %s\n", resultstr, opnstr1);
            break;
        case RETURN:
            if (h->result.kind)
                printf("  RETURN %s\n", resultstr);
            else
                printf("  RETURN\n");
            break;
        }
        h = h->next;
    } while (h != head);
}
void semantic_error(int line, char *msg1, char *msg2) {
    //这里可以只收集错误信息，最后一次显示
    printf("在%d行,%s %s\n", line, msg1, msg2);
}
void prn_symbol() { //显示符号表 
    printf("\n%-6s %-6s %-6s  %-6s %-4s %-6s\n", "var", "alias", "level", "type", "flag", "offset");
    for (symbol_t* symbol = scope_stack.symbol_tables[scope_stack.idx]; symbol != NULL; symbol = symbol->hh.next)
        printf("%-6s %-6s %-6d  %-6s %-4c %-6d\n", symbol->name, \
               symbol->alias, symbol->level, \
               symbol->type == INT ? "int" : "float", \
               symbol->flag, symbol->offset);
}

symbol_t* searchSymbolTable(char *name) {
    symbol_t* symbol_table;
    for (int i = scope_stack.idx; i >= 0; i--) {
        HASH_FIND_STR(scope_stack.symbol_tables[i], name, symbol_table);
        if (symbol_table != NULL)
            return symbol_table;
    }
    return NULL;
}

symbol_t* fillSymbolTable(char *name, char *alias, int level, int type, char flag, int offset) {
    symbol_t* symbol;
    HASH_FIND_STR(scope_stack.symbol_tables[scope_stack.idx], name, symbol);
    if (symbol != NULL)
        return NULL;
    symbol = new_symbol(name, alias, level, type, flag, offset);
    HASH_ADD_STR(scope_stack.symbol_tables[scope_stack.idx], name, symbol);
    return symbol;
}

//填写临时变量到符号表，返回临时变量在符号表中的位置
symbol_t* fill_Temp(char *name, int level, int type, char flag, int offset) {
    return new_symbol("", name, level, type, flag, offset);
}



int LEV = 0;    //层号
int func_size;  //1个函数的活动记录大小

void ext_var_list(struct node *T) { //处理变量列表
    int rtn, num = 1;
    int width = T->type == INT ? 4 : 8; //一个变量宽度
    struct opn opn1, opn2, result;
    symbol_t* symbol;
    switch (T->kind) {
    case VAR_DEC_LIST:
        semantic_Analysis(T);
        T->ptr[0]->offset = T->offset;
        break;
    case ID:
        symbol = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset); //最后一个变量名
        if (symbol == NULL)
            semantic_error(T->pos, T->type_id, "变量重复定义");
        else T->place = symbol;
        T->num = 1;
        T->width = width;
        break;
    case ASSIGNOP:
        T->ptr[0]->type = T->type;
        T->ptr[0]->offset = T->offset;
        ext_var_list(T->ptr[0]);
        T->num = T->ptr[0]->num + 1;
        T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
        Exp(T->ptr[1]);
        opn1.kind = ID;
        strcpy(opn1.id, T->ptr[1]->place->alias);
        result.kind = ID;
        strcpy(result.id, T->ptr[0]->place->alias);
        T->code = merge(3, T->code, T->ptr[1]->code, genIR(ASSIGNOP, opn1, opn2, result));
        T->width = T->ptr[0]->offset + T->ptr[1]->width;    // TODO: check
        break;
    }
}

int  match_param(symbol_t* symbol, struct node *T) {
    int num = symbol->paramnum;
    int type1, type2, pos = -1;
    if (num == 0 && T == NULL) return 1;
    for (symbol_t* s = symbol; s != NULL, num > 0; s = s->hh.next, num--) {
        if (!T) {
            semantic_error(pos, "", "函数调用参数太少");
            return 0;
        }
        pos = T->pos;
        type1 = s->type; //形参类型
        type2 = T->ptr[0]->type;
        if (type1 != type2) {
            semantic_error(pos, "", "参数类型不匹配");
            return 0;
        }
        T = T->ptr[1];
    }
    if (T) { //num个参数已经匹配完，还有实参表达式
        semantic_error(T->pos, "", "函数调用参数太多");
        return 0;
    }
    return 1;
}

void boolExp(struct node *T) { //布尔表达式，参考文献[2]p84的思想
    struct opn opn1, opn2, result;
    int op;
    int rtn;
    symbol_t* symbol;
    if (T) {
        switch (T->kind) {
        case INT:
            if (T->type_int != 0)
                T->code = genGoto(T->Btrue);
            else T->code = genGoto(T->Bfalse);
            T->width = 0;
            break;
        case FLOAT:
            if (T->type_float != 0.0)
                T->code = genGoto(T->Btrue);
            else T->code = genGoto(T->Bfalse);
            T->width = 0;
            break;
        case ID:    //查符号表，获得符号表中的位置，类型送type
            symbol = searchSymbolTable(T->type_id);
            if (symbol == NULL)
                semantic_error(T->pos, T->type_id, "变量未定义");
            if (symbol->flag == 'F')
                semantic_error(T->pos, T->type_id, "是函数名，类型不匹配");
            else {
                opn1.kind = ID;
                strcpy(opn1.id, symbol->alias);
                opn1.offset = symbol->offset;
                opn2.kind = INT;
                opn2.const_int = 0;
                result.kind = ID;
                strcpy(result.id, T->Btrue);
                T->code = genIR(JNE, opn1, opn2, result);
                T->code = merge(2, T->code, genGoto(T->Bfalse));
            }
            T->width = 0;
            break;
        case CMP: //处理关系运算表达式,2个操作数都按基本表达式处理
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            Exp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
            opn1.kind = ID;
            strcpy(opn1.id, T->ptr[0]->place->alias);
            opn1.offset = T->ptr[0]->place->offset;
            opn2.kind = ID;
            strcpy(opn2.id, T->ptr[1]->place->alias);
            opn2.offset = T->ptr[1]->place->offset;
            result.kind = ID;
            if (strcmp(T->Btrue, "fall") != 0 && strcmp(T->Bfalse, "fall") != 0) {
                strcpy(result.id, T->Btrue);
                T->code = genIR(str_to_op(T->type_id, 0), opn1, opn2, result);
                T->code = merge(2, T->code, genGoto(T->Bfalse));
            } else if (strcmp(T->Btrue, "fall") != 0) {
                strcpy(result.id, T->Btrue);
                T->code = genIR(str_to_op(T->type_id, 0), opn1, opn2, result);
            } else if (strcmp(T->Bfalse, "fall") != 0) {
                strcpy(result.id, T->Bfalse);
                T->code = genIR(str_to_op(T->type_id, 1), opn1, opn2, result);
            } else
                T->code = NULL;
            T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, T->code);
            break;
        case AND:
        case OR:
            if (T->kind == AND) {
                strcpy(T->ptr[0]->Btrue, "fall");
                if (strcmp(T->Bfalse, "fall") != 0)
                    strcpy(T->ptr[0]->Bfalse, T->Bfalse);
                else
                    strcpy(T->ptr[0]->Bfalse, newLabel());
            } else {
                if (strcmp(T->Btrue, "fall") != 0)
                    strcpy(T->ptr[0]->Btrue, T->Btrue);
                else
                    strcpy(T->ptr[0]->Btrue, newLabel());
                strcpy(T->ptr[0]->Bfalse, "fall");
            }
            strcpy(T->ptr[1]->Btrue, T->Btrue);
            strcpy(T->ptr[1]->Bfalse, T->Bfalse);
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            boolExp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
            if (T->kind == AND) {
                if (strcmp(T->Bfalse, "fall") != 0)
                    T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
                else
                    T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genLabel(T->ptr[1]->Bfalse));
            } else   {
                if (strcmp(T->Btrue, "fall") != 0)
                    T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
                else
                    T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genLabel(T->ptr[0]->Btrue));
            }
            break;
        case NOT:
            strcpy(T->ptr[0]->Btrue, T->Bfalse);
            strcpy(T->ptr[0]->Bfalse, T->Btrue);
            boolExp(T->ptr[0]);
            T->code = T->ptr[0]->code;
            break;
        }
    }
}


void Exp(struct node *T) {  // TODO: check
    //处理基本表达式，参考文献[2]p82的思想
    int rtn, num, width;
    struct node *T0;
    struct opn opn1, opn2, result;
    symbol_t* symbol;
    if (T) {
        switch (T->kind) {
        case ID:    //查符号表，获得符号表中的位置，类型送type
            symbol = searchSymbolTable(T->type_id);
            if (symbol == NULL)
                semantic_error(T->pos, T->type_id, "变量未定义");
            else if (symbol->flag == 'F')
                semantic_error(T->pos, T->type_id, "是函数名，类型不匹配");
            else {
                T->place = symbol;     //结点保存变量在符号表中的位置
                T->code = NULL;     //标识符不需要生成TAC
                T->type = symbol->type;
                T->offset = symbol->offset;
                T->width = 0; //未再使用新单元
            }
            break;
        case INT:
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
            T->type = INT;
            opn1.kind = INT;
            opn1.const_int = T->type_int;
            result.kind = ID;
            strcpy(result.id, T->place->alias);
            result.offset = T->offset;
            T->code = genIR(ASSIGNOP, opn1, opn2, result);
            T->width = 4;
            break;
        case FLOAT:
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset); //为浮点常量生成一个临时变量
            T->type = FLOAT;
            opn1.kind = FLOAT;
            opn1.const_float = T->type_float;
            result.kind = ID;
            strcpy(result.id, T->place->alias);
            result.offset = T->offset;
            T->code = genIR(ASSIGNOP, opn1, opn2, result);
            T->width = 4;
            break;
        case ASSIGNOP:
            if (T->ptr[0]->kind != ID)
                semantic_error(T->pos, "", "赋值语句需要左值");
            else {
                Exp(T->ptr[0]);   //处理左值，例中仅为变量
                T->ptr[1]->offset = T->offset;
                Exp(T->ptr[1]);
                T->type = T->ptr[0]->type;
                T->width = T->ptr[1]->width;
                T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
                opn1.kind = ID;
                strcpy(opn1.id, T->ptr[1]->place->alias); //右值一定是个变量或临时变量
                opn1.offset = T->ptr[1]->place->offset;
                result.kind = ID;
                strcpy(result.id, T->ptr[0]->place->alias);
                result.offset = T->ptr[0]->place->offset;
                T->code = merge(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
            }
            break;
        case AND:   //按算术表达式方式计算布尔值，未写完
        case OR:    //按算术表达式方式计算布尔值，未写完
        case CMP: //按算术表达式方式计算布尔值，未写完
            T->type = INT;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->ptr[1]->offset = T->offset + T->ptr[0]->width;
            Exp(T->ptr[1]);
            //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
            //下面的类型属性计算，没有考虑错误处理情况
            if (T->ptr[0]->type == FLOAT || T->ptr[1]->type == FLOAT)
                T->type = FLOAT, T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
            else T->type = INT, T->width = T->ptr[0]->width + T->ptr[1]->width + 2;
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
            opn1.kind = ID;
            strcpy(opn1.id, T->ptr[0]->place->alias);
            opn1.type = T->ptr[0]->type;
            opn1.offset = T->ptr[0]->place->offset;
            opn2.kind = ID;
            strcpy(opn2.id, T->ptr[1]->place->alias);
            opn2.type = T->ptr[1]->type;
            opn2.offset = T->ptr[1]->place->offset;
            result.kind = ID;
            strcpy(result.id, T->place->alias);
            result.type = T->type;
            result.offset = T->place->offset;
            T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genIR(T->kind, opn1, opn2, result));
            T->width = T->ptr[0]->width + T->ptr[1]->width + (T->type == INT ? 4 : 8);
            break;
        case NOT:   //未写完整
            break;
        case UMINUS://未写完整
            break;
        case FUNC_CALL: //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
            symbol = searchSymbolTable(T->type_id);
            if (symbol == NULL) {
                semantic_error(T->pos, T->type_id, "函数未定义");
                break;
            }
            if (symbol->flag != 'F') {
                semantic_error(T->pos, T->type_id, "不是一个函数");
                break;
            }
            T->type = symbol->type;
            width = T->type == INT ? 4 : 8; //存放函数返回值的单数字节数
            if (T->ptr[0]) {
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);       //处理所有实参表达式求值，及类型
                T->width = T->ptr[0]->width + width; //累加上计算实参使用临时变量的单元数
                T->code = T->ptr[0]->code;
            } else {
                T->width = width;
                T->code = NULL;
            }
            match_param(symbol, T->ptr[0]);  //处理所以参数的匹配
            //处理参数列表的中间代码
            T0 = T->ptr[0];
            while (T0) {
                result.kind = ID;
                strcpy(result.id, T0->ptr[0]->place->alias);
                result.offset = T0->ptr[0]->place->offset;
                T->code = merge(2, T->code, genIR(ARG, opn1, opn2, result));
                T0 = T0->ptr[1];
            }
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
            opn1.kind = ID;
            strcpy(opn1.id, T->type_id); //保存函数名
            opn1.offset = rtn; //这里offset用以保存函数定义入口,在目标代码生成时，能获取相应信息
            result.kind = ID;
            strcpy(result.id, T->place->alias);
            result.offset = T->place->offset;
            T->code = merge(2, T->code, genIR(CALL, opn1, opn2, result)); //生成函数调用中间代码
            break;
        case ARGS:      //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            T->code = T->ptr[0]->code;
            if (T->ptr[1]) {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                Exp(T->ptr[1]);
                T->width += T->ptr[1]->width;
                T->code = merge(2, T->code, T->ptr[1]->code);
            }
            break;
        }
    }
}

void semantic_Analysis(struct node *T) {
    //对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查和TAC生成（语句部分）
    int rtn, num, width;
    struct node *T0;
    struct opn opn1, opn2, result;
    symbol_t* symbol;
    if (T) {
        switch (T->kind) {
        case EXT_STMT_LIST:
            if (!T->ptr[0]) break;
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]);    //访问外部定义列表中的第一个
            T->code = T->ptr[0]->code;
            if (T->ptr[1]) {
                T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
                T->code = merge(2, T->code, T->ptr[1]->code);
            }
            break;
        case EXT_VAR_DEF:   //处理外部说明,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            T->type = T->ptr[1]->type = resolve_type(T->ptr[0]->type_id);
            T->ptr[1]->type = resolve_type(T->ptr[0]->type_id); //确定变量序列各变量类型
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]);
            T->code = T->ptr[1]->code;
            T->width = T->ptr[1]->width;
            break;
        case FUNC_DEF:      //填写函数定义信息到符号表
            T->ptr[0]->type = resolve_type(T->ptr[0]->ptr[1]->type_id); //获取函数返回类型送到含函数名、参数的结点
            T->width = 0;   //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset = DX; //设置局部变量在活动记录中的偏移量初值
            T->ptr[0]->func_def = 1;
            semantic_Analysis(T->ptr[0]); //处理函数名和参数结点部分，这里不考虑用寄存器传递参数
            T->ptr[0]->place->func_def = 1;
            T->offset += T->ptr[0]->width; //用形参单元宽度修改函数局部变量的起始偏移量
            T->ptr[1]->offset = T->offset;
            strcpy(T->ptr[1]->Snext, newLabel()); //函数体语句执行结束后的位置属性
            semantic_Analysis(T->ptr[1]);         //处理函数体结点
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            T->ptr[0]->place->offset = T->offset + T->ptr[1]->width;
            T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genLabel(T->ptr[1]->Snext));     //函数体的代码作为函数的代码
            break;
        case FUNC_DEC:      //根据返回类型，函数名填写符号表
            T->type = T->ptr[1]->type;
            symbol = searchSymbolTable(T->type_id);
            if (symbol != NULL) {
                if (symbol->func_def == 1 && T->func_def == 1)
                    semantic_error(T->pos, T->type_id, "函数声明重复定义");
                else if (symbol->func_def == 1 && T->func_def == 0)
                    semantic_error(T->pos, T->type_id, "函数声明在定义之后");
                else if (symbol->func_def == 0 && T->func_def == 0)
                    semantic_error(T->pos, T->type_id, "函数重复声明");
                else {
                    symbol->func_def = 1;
                    T->place = symbol;
                }
                break;
            }
            symbol = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'F', 0); //函数不在数据区中分配单元，偏移量为0
            T->place = symbol;
            result.kind = ID;
            strcpy(result.id, T->type_id);
            // result.offset = rtn;
            T->code = genIR(FUNCTION, opn1, opn2, result); //生成中间代码：FUNCTION 函数名
            T->offset = DX; //设置形式参数在活动记录中的偏移量初值
            if (T->ptr[0]) { //判断是否有参数
                T->ptr[0]->offset = T->offset;
                semantic_Analysis(T->ptr[0]);  //处理函数参数列表
                T->width = T->ptr[0]->width;
                symbol->paramnum = T->ptr[0]->num;
                T->code = merge(2, T->code, T->ptr[0]->code); //连接函数名和参数代码序列
            } else symbol->paramnum = 0, T->width = 0;
            break;
        case PARAM_LIST:    //处理函数形式参数列表 TODO: check
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]);
            if (T->ptr[1]) {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);
                T->num = T->ptr[0]->num + T->ptr[1]->num;    //统计参数个数
                T->width = T->ptr[0]->width + T->ptr[1]->width; //累加参数单元宽度
                T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code); //连接参数代码
            } else {
                T->num = T->ptr[0]->num;
                T->width = T->ptr[0]->width;
                T->code = T->ptr[0]->code;
            }
            break;
        case PARAM_DEC:    // TODO: check
            symbol = fillSymbolTable(T->ptr[1]->type_id, newAlias(), 1, T->ptr[0]->type, 'P', T->offset);
            if (symbol == NULL)
                semantic_error(T->ptr[1]->pos, T->ptr[1]->type_id, "参数名重复定义");
            else T->ptr[1]->place = symbol;
            T->num = 1;     //参数个数计算的初始值
            T->width = T->ptr[0]->type == INT ? 4 : 8; //参数宽度
            result.kind = ID;
            strcpy(result.id, symbol->alias);
            result.offset = T->offset;
            T->code = genIR(PARAM, opn1, opn2, result); //生成：FUNCTION 函数名
            break;
        case COMP_STM:
            LEV++;
            //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            // symbol_scope_TX.TX[symbol_scope_TX.top++] = symbolTable.index;
            scope_stack.symbol_tables[++scope_stack.idx] = NULL;
            T->width = 0;
            T->code = NULL;
            if (T->ptr[0]) {
                T->ptr[0]->offset = T->offset;
                strcpy(T->ptr[0]->Snext, T->Snext); //S.next属性向下传递
                semantic_Analysis(T->ptr[0]);  //处理该层的局部变量DEF_LIST
                T->width += T->ptr[0]->width;
                T->code = T->ptr[0]->code;
            }
            // if (T->ptr[1]) {
            //     T->ptr[1]->offset = T->offset + T->width;
            //     strcpy(T->ptr[1]->Snext, T->Snext); //S.next属性向下传递
            //     semantic_Analysis(T->ptr[1]);       //处理复合语句的语句序列
            //     T->width += T->ptr[1]->width;
            //     T->code = merge(2, T->code, T->ptr[1]->code);
            // }
            prn_symbol();       //c在退出一个符合语句前显示的符号表
            LEV--;    //出复合语句，层号减1
            // symbolTable.index = symbol_scope_TX.TX[--symbol_scope_TX.top]; //删除该作用域中的符号
            scope_stack.idx--;
            break;
        case COMP_LIST:
            if (T->ptr[1] && (T->ptr[0]->kind == IF_THEN || T->ptr[0]->kind == IF_THEN_ELSE || T->ptr[0]->kind == WHILE))
                strcpy(T->ptr[0]->Snext, newLabel());
            else
                strcpy(T->ptr[0]->Snext, T->Snext);
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]);
            T->code = T->ptr[0]->code;
            T->width += T->ptr[0]->width;
            if (T->ptr[1]) {
                strcpy(T->ptr[1]->Snext, T->Snext);
                T->ptr[1]->offset = T->offset + T->width;
                semantic_Analysis(T->ptr[1]);       //处理复合语句的语句序列
                T->width += T->ptr[1]->width;
                if (T->ptr[0]->kind == VAR_DEC_STMT || T->ptr[0]->kind == RETURN || T->ptr[0]->kind == EXP_STMT || T->ptr[0]->kind == COMP_STM)
                    T->code = merge(2, T->code, T->ptr[1]->code);
                else
                    T->code = merge(3, T->code, genLabel(T->ptr[0]->Snext), T->ptr[1]->code);
            }
            break;
        case VAR_DEC_LIST:
            T->code = NULL;
            T->ptr[0]->type = T->type;
            T->ptr[0]->offset = T->offset;
            ext_var_list(T->ptr[0]);   //处理一个局部变量定义
            T->code = T->ptr[0]->code;
            T->width = T->ptr[0]->width;
            if (T->ptr[1]) {
                T->ptr[1]->type = T->type;
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                ext_var_list(T->ptr[1]);   //处理剩下的局部变量定义
                T->code = merge(2, T->code, T->ptr[1]->code);
                T->width += T->ptr[1]->width;
            }
            break;
        case VAR_DEC_STMT://处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
            T->ptr[1]->type = resolve_type(T->ptr[0]->type_id); //确定变量序列各变量类型
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]);
            T->code = T->ptr[1]->code;
            T->width = T->ptr[1]->width;
            break;
        // case STM_LIST:  // TODO: check
        //     if (!T->ptr[0]) {
        //         T->code = NULL;    //空语句序列
        //         T->width = 0;
        //         break;
        //     }
        //     if (T->ptr[1]) //2条以上语句连接，生成新标号作为第一条语句结束后到达的位置
        //         strcpy(T->ptr[0]->Snext, newLabel());
        //     else     //语句序列仅有一条语句，S.next属性向下传递
        //         strcpy(T->ptr[0]->Snext, T->Snext);
        //     T->ptr[0]->offset = T->offset;
        //     semantic_Analysis(T->ptr[0]);
        //     T->code = T->ptr[0]->code;
        //     T->width = T->ptr[0]->width;
        //     if (T->ptr[1]) {    //2条以上语句连接,S.next属性向下传递
        //         strcpy(T->ptr[1]->Snext, T->Snext);
        //         T->ptr[1]->offset = T->offset; //顺序结构共享单元方式
        //         // T->ptr[1]->offset = T->offset + T->ptr[0]->width; //顺序结构顺序分配单元方式
        //         semantic_Analysis(T->ptr[1]);
        //         //序列中第1条为表达式语句，返回语句，复合语句时，第2条前不需要标号
        //         if (T->ptr[0]->kind == RETURN || T->ptr[0]->kind == EXP_STMT || T->ptr[0]->kind == COMP_STM)
        //             T->code = merge(2, T->code, T->ptr[1]->code);
        //         else
        //             T->code = merge(3, T->code, genLabel(T->ptr[0]->Snext), T->ptr[1]->code);
        //         if (T->ptr[1]->width > T->width) T->width = T->ptr[1]->width; //顺序结构共享单元方式
        //         T->width += T->ptr[1]->width; //顺序结构顺序分配单元方式
        //     }
        //     break;
        case IF_THEN:
            strcpy(T->ptr[0]->Btrue, "fall");
            strcpy(T->ptr[1]->Snext, T->Snext);
            strcpy(T->ptr[0]->Bfalse, T->Snext);
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            semantic_Analysis(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
            T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
            break;  //控制语句都还没有处理offset和width属性
        case IF_THEN_ELSE:
            strcpy(T->ptr[0]->Btrue, "fall");
            strcpy(T->ptr[0]->Bfalse, newLabel());
            T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            strcpy(T->ptr[1]->Snext, T->Snext);
            semantic_Analysis(T->ptr[1]);
            if (T->width < T->ptr[1]->width) T->width = T->ptr[1]->width;
            strcpy(T->ptr[2]->Snext, T->Snext);
            semantic_Analysis(T->ptr[2]);
            if (T->width < T->ptr[2]->width) T->width = T->ptr[2]->width;
            T->code = merge(5, T->ptr[0]->code, T->ptr[1]->code, \
                            genGoto(T->Snext), genLabel(T->ptr[0]->Bfalse), T->ptr[2]->code);
            break;
        case WHILE:
            strcpy(T->ptr[0]->Btrue, "fall");
            strcpy(T->ptr[0]->Bfalse, T->Snext);
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            strcpy(T->ptr[1]->Snext, newLabel());
            semantic_Analysis(T->ptr[1]);      //循环体
            if (T->width < T->ptr[1]->width) T->width = T->ptr[1]->width;
            T->code = merge(4, genLabel(T->ptr[1]->Snext), T->ptr[0]->code, \
                            T->ptr[1]->code, genGoto(T->ptr[1]->Snext));
            break;
        case EXP_STMT:
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]);
            T->code = T->ptr[0]->code;
            T->width = T->ptr[0]->width;
            break;
        case RETURN:
            if (T->ptr[0]) {
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);
                // num = symbolTable.index;
                // do num--;
                // while (symbolTable.symbols[num].flag != 'F');
                // if (T->ptr[0]->type != symbolTable.symbols[num].type) {
                //     semantic_error(T->pos, "返回值类型错误", "");
                //     T->width = 0;
                //     T->code = NULL;
                //     break;
                // }
                symbol_t* func_symbol = NULL;
                for (num = scope_stack.idx; num >= 0; num--) {
                    func_symbol = NULL;
                    for (symbol = scope_stack.symbol_tables[num]; symbol != NULL; symbol = symbol->hh.next) {
                        if (symbol->flag == 'F')
                            func_symbol = symbol;
                    }
                    if (func_symbol != NULL)
                        break;
                }
                if (func_symbol == NULL || T->ptr[0]->type != func_symbol->type) {
                    if (func_symbol == NULL)
                        semantic_error(T->pos, "在函数外return", "");
                    else
                        semantic_error(T->pos, "返回值类型错误", "");
                    T->width = 0;
                    T->code = NULL;
                    break;
                }
                T->width = T->ptr[0]->width;
                result.kind = ID;
                strcpy(result.id, T->ptr[0]->place->alias);
                result.offset = T->ptr[0]->place->offset;
                T->code = merge(2, T->ptr[0]->code, genIR(RETURN, opn1, opn2, result));
            } else {
                T->width = 0;
                result.kind = 0;
                T->code = genIR(RETURN, opn1, opn2, result);
            }
            break;
        case ID:
        case INT:
        case FLOAT:
        case ASSIGNOP:
        case AND:
        case OR:
        case CMP:
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
        case NOT:
        case UMINUS:
        case FUNC_CALL:
            Exp(T);          //处理基本表达式
            break;
        }
    }
}

void semantic_Analysis0(struct node *T) {
    // symbolTable.index = 0;
    // fillSymbolTable("read", "", 0, INT, 'F', 4);
    // symbolTable.symbols[0].paramnum = 0; //read的形参个数
    // fillSymbolTable("write", "", 0, INT, 'F', 4);
    // symbolTable.symbols[2].paramnum = 1;
    // fillSymbolTable("x", "", 1, INT, 'P', 12);
    // symbol_scope_TX.TX[0] = 0; //外部变量在符号表中的起始序号为0
    // symbol_scope_TX.top = 1;
    scope_stack.idx = 0;
    scope_stack.symbol_tables[0] = NULL;
    T->offset = 0;            //外部变量在数据区的偏移量
    semantic_Analysis(T);
    prn_symbol();
    prnIR(T->code);
    // objectCode(T->code);
}