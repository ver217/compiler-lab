#include "Node.h"
#include <stdlib.h>

PEXP new_node(enum node_kind kind, PEXP l_child, PEXP r_child) {
    PEXP node = (PEXP)malloc(sizeof(struct Exp));
    node->kind = kind;
    node->ptr.pExp1 = l_child;
    node->ptr.pExp2 = r_child;
    return node;
}