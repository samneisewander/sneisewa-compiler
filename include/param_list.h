
#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <stdio.h>

#include "type.h"

struct expr;

struct param_list {
    char* name;
    struct type* type;
    struct symbol* symbol;
    struct param_list* next;
};

struct param_list* param_list_create(char* name, struct type* type, struct param_list* next);

void param_list_print(struct param_list* a, FILE* fp);

/**
 * Enters a new variable declaration for each parameter of a function into the symbol table, so that those definitions are available to the code of a function.
 */
void param_list_resolve(struct param_list* a);

/**
 * Iteratively release nodes in param_list `a`. If `a` is `NULL`, do nothing. Note that the symbol structures associated with each node are not released.
 */
void param_list_delete(struct param_list* a);

#endif