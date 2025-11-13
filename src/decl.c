/* decl.c */

#include "decl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "expr.h"
#include "param_list.h"
#include "scope.h"
#include "stmt.h"
#include "symbol.h"
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

    symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    d->symbol = symbol_create(kind, d->type, d->name);
    struct symbol* binding = scope_lookup_current(d->name);
    expr_resolve(d->value);

    if (!binding) {
        /* Case 1: (LEGAL) this name has never been bound. */
        if (d->code) {
            d->symbol->func_defined = true;
            scope_enter(SCOPE_FUNCTION);
            param_list_resolve(d->type->params);
            scope_enter(SCOPE_FUNCTION);
            stmt_resolve(d->code);
            scope_exit();
            scope_exit();
        }
        scope_bind(d->name, d->symbol);
    } else {
        if (d->type->kind == TYPE_FUNCTION && binding->type->kind == TYPE_FUNCTION) {
            if (binding->func_defined) {
                if (d->code) {
                    /* Case 7: (ERROR) declaration is a function definition, and this name has been bound to a function definition */
                    fprintf(stderr, "resolver: competing definitions for function %s\n", d->name);
                    set_error(true);
                    // TODO: Destroy symbol.
                } else {
                    /* Case 5: (LEGAL, if types match) declaration is a function prototype, and this name has already been bound to a function definition */
                    // TODO: Destroy symbol.
                    d->symbol = binding;
                }
            } else {
                if (d->code) {
                    /* Case 6: (LEGAL, if types match) declaration is a function definition, and this name has only been bound to a function prototype */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    d->symbol = binding;
                    d->symbol->func_defined = true;
                    scope_enter(SCOPE_FUNCTION);
                    param_list_resolve(d->type->params);
                    scope_enter(SCOPE_FUNCTION);
                    stmt_resolve(d->code);
                    scope_exit();
                    scope_exit();
                } else {
                    /* Case 4: (LEGAL, if types match) declaration is a function prototype, and this name has not been bound to a function definition */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    d->symbol = binding;
                }
            }
        } else {
            /* Case 2: (ERROR) declaration is not a function, and this name has already been bound. */
            /* Case 3: (ERROR) declaration is a function, and this name has already been bound to something other than a function. */
            fprintf(stderr, "resolver: redeclaration of variable %s\n", d->name);
            set_error(true);
        }
    }

    decl_resolve(d->next);
}