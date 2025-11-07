/* decl.c */

#include "decl.h"

#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "type.h"

// Declarations
struct decl* decl_create(char* name, struct type* type, struct expr* value, struct stmt* code, struct decl* next) {
    struct decl* d = malloc(sizeof(*d));

    d->name = name;
    d->type = type;
    d->value = value;
    d->code = code;
    d->next = next;

    return d;
}

void decl_print(struct decl* d, int indent) {
    if (!d) {
        return;
    }

    for (int i = 0; i < indent; i++) putc('\t', stdout);
    printf("%s: ", d->name);
    type_print(d->type);

    if (d->value) {
        printf(" = ");
        expr_print(d->value);
        putc(';', stdout);
    } else if (d->code) {
        printf(" = \n");
        stmt_print(d->code, indent);
    } else {
        putc(';', stdout);
    }

    putc('\n', stdout);
    decl_print(d->next, indent);
}

void decl_resolve(struct decl* d) {
    if (!d) return;
    // symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    // d->symbol = symbol_create(kind, d->type, d->name);
    // expr_resolve(d->value);
    // scope_bind(d->name, d->symbol);
    // if (d->code) {
    //     scope_enter();
    //     param_list_resolve(d->type->params);
    //     stmt_resolve(d->code);
    //     scope_exit();
    // }
    // decl_resolve(d->next);
}