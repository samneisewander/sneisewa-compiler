/* expr.c */

#include "param_list.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "scope.h"

struct param_list* param_list_create(char* name, struct type* type, struct param_list* next) {
    struct param_list* p = calloc(1, sizeof(*p));
    p->name = name;
    p->type = type;
    p->next = next;
    p->symbol = NULL;
    return p;
}

void param_list_print(struct param_list* a) {
    if (!a) {
        return;
    }

    printf("%s: ", a->name);
    type_print(a->type);

    if (a->next) {
        printf(", ");
        param_list_print(a->next);
    } else {
        return;
    }
}

void param_list_resolve(struct param_list* a) {
    if (!a) return;
    scope_bind(a->name, symbol_create(SYMBOL_PARAM, a->type, a->name));
    param_list_resolve(a->next);
}