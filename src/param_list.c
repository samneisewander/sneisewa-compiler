/* expr.c */

#include "param_list.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "scope.h"

struct param_list* param_list_create(char* name, struct type* type, struct param_list* next) {
    struct param_list* p = calloc(1, sizeof(*p));
    p->name = name;
    p->type = type;
    p->next = next;
    p->symbol = NULL;
    return p;
}

void param_list_print(struct param_list* a, FILE* fp) {
    if (!a) {
        return;
    }

    printf("%s: ", a->name);
    type_print(a->type, fp);

    if (a->next) {
        printf(", ");
        param_list_print(a->next, fp);
    } else {
        return;
    }
}

void param_list_resolve(struct param_list* a) {
    if (!a) return;
    if (scope_lookup_current(a->name)) {
        fprintf(stderr, "resolver: duplicate parameter %s in function.\n", a->name);
        set_error(true);
    } else {
        scope_bind(a->name, symbol_create(SYMBOL_PARAM, a->type, a->name));
    }
    param_list_resolve(a->next);
}

void param_list_delete(struct param_list* a) {
    if (!a) return;

    struct param_list* curr = a;
    struct param_list* next = curr->next;

    while (next) {
        free(curr->name);
        type_delete(curr->type);
        free(curr);
        curr = next;
        next = next->next;
    }
}
