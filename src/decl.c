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
    type_print(d->type, stdout);

    if (d->value) {
        printf(" = ");
        expr_print(d->value, stdout);
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
        scope_bind(d->name, d->symbol);
        if (d->code) {
            d->symbol->func_defined = true;
            scope_enter(SCOPE_FUNCTION);
            param_list_resolve(d->type->params);
            scope_enter(SCOPE_FUNCTION);
            stmt_resolve(d->code);
            scope_exit();
            scope_exit();
        }
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
                    if (type_equals(d->type, binding->type)) {
                        d->symbol = binding;
                    }
                    // TODO: Destroy symbol.
                }
            } else {
                if (d->code) {
                    /* Case 6: (LEGAL, if types match) declaration is a function definition, and this name has only been bound to a function prototype */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    if (type_equals(d->type, binding->type)) {
                        d->symbol = binding;
                        d->symbol->func_defined = true;
                        scope_enter(SCOPE_FUNCTION);
                        param_list_resolve(d->type->params);
                        scope_enter(SCOPE_FUNCTION);
                        stmt_resolve(d->code);
                        scope_exit();
                        scope_exit();
                    }
                    // TODO: Destroy symbol.

                } else {
                    /* Case 4: (LEGAL, if types match) declaration is a function prototype, and this name has not been bound to a function definition */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    d->symbol = binding;
                }
            }
        } else {
            /* Case 2: (ERROR) declaration is not a function, and this name has already been bound. */
            /* Case 3: (ERROR) declaration is a function, and this name has already been bound to something other than a function. */
            // TODO: destroy symbol
            fprintf(stderr, "resolver: redeclaration of variable %s\n", d->name);
            set_error(true);
        }
    }

    decl_resolve(d->next);
}

void decl_typecheck(struct decl* d) {
    if (!d) return;

    /**
     * The following “oddities” should be flagged as errors, even though it is possible to express them syntactically:
     * Functions that return arrays or other functions.
     * Functions as parameters to other functions.
     * Arrays of functions.
     * Non-constant initializers for global variables.
     * Array initializers for local variables.
     */

    if (d->symbol->kind == SYMBOL_GLOBAL) {
        // Prevent non-constant initializers for global variables
        switch (d->type->kind) {
            case TYPE_ARRAY:
            case TYPE_CARRAY:
                if (d->type->array_size->kind != EXPR_INT) {
                    fprintf(stderr, "typecheck error: global array %s cannot have non-constant size (", d->name);
                    expr_print(d->type->array_size, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                break;
            case TYPE_AUTO:
            case TYPE_BOOLEAN:
            case TYPE_CHARACTER:
            case TYPE_INTEGER:
            case TYPE_DOUBLE:
            case TYPE_STRING:
                if (d->value && d->value->kind < EXPR_NAME) {
                    fprintf(stderr, "typecheck error: global variable %s cannot have non-constant initializer (", d->name);
                    expr_print(d->value, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                break;
            default:
                // Shoo, gcc errors!
                break;
        }
    }

    // Check for size 0 arrays
    if (d->type->kind == TYPE_ARRAY || d->type->kind == TYPE_CARRAY) {
        if (d->type->array_size->kind == EXPR_INT && d->type->array_size->int_literal == 0) {
            fprintf(stderr, "typecheck error: array %s cannot have size of 0 (", d->name);
            expr_print(d->type->array_size, stderr);
            fprintf(stderr, ")\n");
            set_error(true);
        }
    }

    if (d->value) {
        struct type* t = expr_typecheck(d->value);

        /**
         * Infer type of symbol from initializing expression. Do not allow auto to be initialized with auto.
         */
        if (d->type->kind == TYPE_AUTO) {
            if (t->kind == TYPE_AUTO) {
                fprintf(stderr, "typecheck error: auto (%s) cannot be initialized with auto (", d->name);
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (t->kind == TYPE_VOID) {
                fprintf(stderr, "typecheck error: auto (%s) cannot be initialized with void (", d->name);
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else {
                type_delete(d->type);
                d->type = type_copy(t);
            }
        }

        if (d->type->kind == TYPE_ARRAY || d->type->kind == TYPE_CARRAY) {
            if (d->symbol->kind == SYMBOL_LOCAL) {
                fprintf(stderr, "typecheck error: cannot initialize local array %s\n", d->name);
                set_error(true);
            } else if (d->type->subtype->kind == TYPE_AUTO) {
                type_delete(d->type->subtype);
                d->type->subtype = type_copy(t);
            } else if (!type_equals(t->subtype, d->symbol->type)) {
                fprintf(stderr, "typecheck error: decl of type ");
                type_print(d->type, stderr);
                fprintf(stderr, " (%s) cannot be initialized with value of type ", d->symbol->name);
                type_print(t->subtype, stderr);
                fprintf(stderr, " (");
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
        } else if (!type_equals(t, d->symbol->type)) {
            fprintf(stderr, "typecheck error: decl of type ");
            type_print(d->type, stderr);
            fprintf(stderr, " (%s) cannot be initialized with value of type ", d->symbol->name);
            type_print(t, stderr);
            fprintf(stderr, " (");
            expr_print(d->value, stderr);
            fprintf(stderr, ")\n");
            set_error(true);
        }
    }

    if (d->code) {
        stmt_typecheck(d->code, d->symbol);
        switch (d->type->subtype->kind) {
            case TYPE_AUTO:
                fprintf(stderr, "typecheck error: cannot infer type of automatically typed function (%s) since it has no return statement.\n", d->name);
                set_error(true);
                break;
            case TYPE_ARRAY:
            case TYPE_CARRAY:
            case TYPE_FUNCTION:
                fprintf(stderr, "typecheck error: cannot return %s type from function (%s).\n", ttos(d->type->subtype), d->name);
                set_error(true);
                break;
            default:
                // Shoo, gcc warnings!
                break;
        }
    }

    decl_typecheck(d->next);
}