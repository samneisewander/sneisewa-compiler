/* expr.c */

#include "expr.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoder.h"
#include "error.h"
#include "scope.h"

/**
 * - Cannot print main
 * - Cannot print function with return type auto
 * - Cannot print auto
 * -
 */

struct expr* expr_create(expr_t kind, struct expr* left, struct expr* right) {
    struct expr* e = calloc(1, sizeof(*e));
    e->kind = kind;
    e->left = left;
    e->right = right;
    return e;
}

struct expr* expr_create_name(char* n) {
    struct expr* e = calloc(1, sizeof(*e));
    e->name = n;
    e->kind = EXPR_NAME;
    return e;
}

struct expr* expr_create_integer_literal(int64_t c) {
    struct expr* e = calloc(1, sizeof(*e));
    e->int_literal = c;
    e->kind = EXPR_INT;
    return e;
}

struct expr* expr_create_double_literal(double c) {
    struct expr* e = calloc(1, sizeof(*e));
    e->double_literal = c;
    e->kind = EXPR_DOUBLE;
    return e;
}

struct expr* expr_create_boolean_literal(bool c) {
    struct expr* e = calloc(1, sizeof(*e));
    e->bool_literal = c;
    e->kind = EXPR_BOOLEAN;
    return e;
}

struct expr* expr_create_char_literal(char c) {
    struct expr* e = calloc(1, sizeof(*e));
    e->char_literal = c;
    e->kind = EXPR_CHAR;
    return e;
}

struct expr* expr_create_string_literal(char* str) {
    struct expr* e = calloc(1, sizeof(*e));
    e->string_literal = str;
    e->kind = EXPR_STRING;
    return e;
}

void expr_print_parens(struct expr* e, FILE* fp) {
    putc('(', stdout);
    expr_print(e, fp);
    putc(')', stdout);
}

void expr_print(struct expr* e, FILE* fp) {
    if (!e) return;

    char buf[BUFSIZ] = {0};

    /**
     * This switch preserves order of operations by checking if the expr on
     * the left has lower precedence than this operation, and wrapping
     * it in parentheses if it does.
     *
     * Also, need to ensure AST node order is preserved, even if it makes
     * no difference mathematically.
     *
     * For example, an expression like 1 + (2 + 3) must preserve the parens
     * around (2 + 3), even though 1 + 2 + 3 evaluates to the same number.
     *
     * This eliminates errata with parsing and printing having to do with
     * floating point operations. (Thanks Prof. Kogge for explaining that)
     *
     * It also makes operations that aren't commutative behave correctly.
     */
    switch (e->kind) {
        case EXPR_ASSIGNMENT:
            expr_print(e->left, fp);
            fprintf(fp, "=");
            expr_print(e->right, fp);
            break;
        case EXPR_LOGICAL_OR:
            if (e->left->kind < EXPR_LOGICAL_OR) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "||");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LOGICAL_OR) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_LOGICAL_AND:
            if (e->left->kind < EXPR_LOGICAL_AND) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "&&");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LOGICAL_AND) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_LESS_THAN:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "<");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LESS_THAN) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_LESS_THAN_OR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "<=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LESS_THAN_OR_EQUAL) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "==");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_EQUAL) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_GREATER_THAN:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, ">");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_GREATER_THAN) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_GREATER_THAN_OR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, ">=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_GREATER_THAN_OR_EQUAL) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_NOT_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "!=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_NOT_EQUAL) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_ADD:
            if (e->left->kind < EXPR_ADD) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "+");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_ADD) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_SUBTRACT:
            if (e->left->kind < EXPR_ADD) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "-");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_SUBTRACT) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_MULTIPLY:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "*");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_MULTIPLY) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_DIVIDE:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "/");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_DIVIDE) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_REMAINDER:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "%%");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_REMAINDER) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_EXPONENT:
            if (e->left->kind < EXPR_NAME && e->left->kind <= EXPR_EXPONENT) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "^");
            if (e->right->kind < EXPR_EXPONENT) {
                expr_print_parens(e->right, fp);
            } else {
                expr_print(e->right, fp);
            }
            break;
        case EXPR_UNARY_NEGATION:
        case EXPR_LOGICAL_NOT:
            fprintf(fp, "!");
            if (e->left->kind < EXPR_UNARY_NEGATION) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            break;
        case EXPR_UNARY_ARRAY_LENGTH:
            fprintf(fp, "#");
            if (e->left->kind < EXPR_UNARY_ARRAY_LENGTH) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            break;
        case EXPR_POSTFIX_INCREMENT:
            if (e->left->kind < EXPR_POSTFIX_INCREMENT) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "++");
            break;
        case EXPR_POSTFIX_DECREMENT:
            if (e->left->kind < EXPR_POSTFIX_INCREMENT) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "--");
            break;
        case EXPR_SUBSCRIPT:
            if (e->left->kind < EXPR_SUBSCRIPT) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "[");
            expr_print(e->right, fp);
            fprintf(fp, "]");
            break;
        case EXPR_CALL:
            if (e->left->kind < EXPR_SUBSCRIPT) {
                expr_print_parens(e->left, fp);
            } else {
                expr_print(e->left, fp);
            }
            fprintf(fp, "(");
            expr_print(e->right, fp);
            fprintf(fp, ")");
            break;
        case EXPR_ARRAY_ELEM:
        case EXPR_ARG:
            expr_print(e->left, fp);
            if (e->right) {
                putc(',', fp);
                expr_print(e->right, fp);
            }
            break;
        case EXPR_ARRAY_INIT:
            putc('{', fp);
            expr_print(e->left, fp);
            putc('}', fp);
            if (e->right) {
                putc(',', fp);
                expr_print(e->right, fp);
            }
            break;
        case EXPR_NAME:
            fprintf(fp, "%s", e->name);
            break;
        case EXPR_INT:
            fprintf(fp, "%ld", e->int_literal);
            break;
        case EXPR_DOUBLE:
            fprintf(fp, "%lf", e->double_literal);
            break;
        case EXPR_BOOLEAN:
            if (e->bool_literal) {
                fprintf(fp, "true");
            } else {
                fprintf(fp, "false");
            }
            break;
        case EXPR_CHAR:
            fprintf(fp, "'\\0x%x'", e->char_literal);
            break;
        case EXPR_STRING:
            string_encode(e->string_literal, buf);
            fprintf(fp, "%s", buf);
            break;
    }
}

void expr_resolve(struct expr* e) {
    if (!e) return;
    if (e->kind == EXPR_NAME) {
        e->symbol = scope_lookup(e->name);
        if (!e->symbol) {
            fprintf(stderr, "resolve error: %s is not defined.\n", e->name);
            set_error(true);
        }
    } else {
        expr_resolve(e->left);
        expr_resolve(e->right);
    }
}

struct expr* expr_copy(struct expr* e) {
    if (!e) return NULL;

    struct expr* e_new = calloc(1, sizeof(*e));

    e_new->kind = e->kind;
    e_new->left = expr_copy(e->left);
    e_new->right = expr_copy(e->right);
    if (e->name) {
        e_new->name = strdup(e->name);
    }
    e_new->int_literal = e->int_literal;
    e_new->bool_literal = e->bool_literal;
    e_new->double_literal = e->double_literal;
    e_new->char_literal = e->char_literal;
    if (e->string_literal) {
        e_new->string_literal = strdup(e->string_literal);
    }
    e_new->symbol = e->symbol;

    return e_new;
}

void expr_delete(struct expr* e) {
    if (!e) return;

    expr_delete(e->left);
    expr_delete(e->right);
    if (e->name) {
        free(e->name);
    }
    if (e->string_literal) {
        free(e->string_literal);
    }

    free(e);
}

struct type* expr_typecheck(struct expr* e) {
    if (!e) return 0;

    /**
     * One thing to note is that by the time this is called, all symbols will have been registered in the symbol table. This means we don't need to check for the existence of a symbol before using it.
     */

    struct type* lt = expr_typecheck(e->left);
    struct type* rt = expr_typecheck(e->right);
    struct type* result;

    switch (e->kind) {
        case EXPR_ASSIGNMENT:
            /* AUTO CAN BE INFERRED HERE */
            if (lt->kind == TYPE_AUTO && rt->kind == TYPE_AUTO) {
                // disallow assigning auto to auto
                fprintf(stderr, "typecheck error: cannot asign value of type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ") to type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (e->left->kind == EXPR_SUBSCRIPT) {
                // determine array subtype
                if (lt->kind == TYPE_AUTO) {
                    struct expr* curr = e->left;
                    while (curr->kind != EXPR_NAME) {
                        curr = curr->left;
                    }
                    struct type* arrType = curr->symbol->type;
                    while (arrType->kind != TYPE_AUTO) {
                        arrType = arrType->subtype;
                    }
                    arrType->kind = rt->kind;
                    lt->kind = rt->kind;
                }
            } else if (e->left->kind == EXPR_NAME) {
                if (lt->kind == TYPE_AUTO) {
                    type_delete(e->left->symbol->type);
                    type_delete(lt);
                    e->left->symbol->type = type_copy(rt);
                    lt = type_copy(rt);
                }
            } else {
                fprintf(stderr, "typecheck error: cannot assign value to type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            if (!type_equals(lt, rt)) {
                fprintf(stderr, "typecheck error: cannot asign value of type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ") to type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (rt->kind == TYPE_ARRAY || rt->kind == TYPE_CARRAY) {
                fprintf(stderr, "typecheck error: cannot assign array outside of initialization (");
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            result = type_copy(lt);
            break;
        case EXPR_LOGICAL_OR:
        case EXPR_LOGICAL_AND:
            /* Only booleans. */
            if (!type_equals(lt, rt) || lt->kind != TYPE_BOOLEAN) {
                fprintf(stderr, "typecheck error: cannot compare type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") to type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            result = type_create(TYPE_BOOLEAN, 0, 0, 0);
            break;
        case EXPR_LESS_THAN:
        case EXPR_LESS_THAN_OR_EQUAL:
        case EXPR_GREATER_THAN:
        case EXPR_GREATER_THAN_OR_EQUAL:
            /* Only integers and doubles. */
            if (!type_equals(lt, rt) || !(lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                fprintf(stderr, "typecheck error: cannot compare type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") to type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            result = type_create(TYPE_BOOLEAN, 0, 0, 0);
            break;
        case EXPR_EQUAL:
        case EXPR_NOT_EQUAL:
            /* Anything except void, array, function. */
            if (!type_equals(lt, rt)) {
                fprintf(stderr, "typecheck error: cannot compare type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") to type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (lt->kind == TYPE_VOID || lt->kind == TYPE_ARRAY || lt->kind == TYPE_FUNCTION) {
                fprintf(stderr, "typecheck error: %s may not appear in comparison operation.\n", ttos(lt));
                set_error(true);
            }
            result = type_create(TYPE_BOOLEAN, 0, 0, 0);
            break;
        case EXPR_ADD:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot add type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") to type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_SUBTRACT:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot subtract type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") from type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_MULTIPLY:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot multiply type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") and type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_DIVIDE:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot divide type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") by type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_REMAINDER:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot divide type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") with type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_EXPONENT:
            if (type_equals(lt, rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE)) {
                result = type_copy(lt);
            } else {
                fprintf(stderr, "typecheck error: cannot exponentiate type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ") by type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_UNARY_NEGATION:
            if (lt->kind == TYPE_BOOLEAN ||
                lt->kind == TYPE_INTEGER ||
                lt->kind == TYPE_DOUBLE) {
                result = type_create(lt->kind, 0, 0, 0);
            } else {
                fprintf(stderr, "typecheck error: cannot negate type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_LOGICAL_NOT:
            if (lt->kind == TYPE_BOOLEAN) {
                result = type_create(TYPE_BOOLEAN, 0, 0, 0);
            } else {
                fprintf(stderr, "typecheck error: cannot logically negate type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_UNARY_ARRAY_LENGTH:
            if (lt->kind == TYPE_CARRAY || lt->kind == TYPE_CARRAY) {
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            } else {
                fprintf(stderr, "typecheck error: cannot take the unary length of type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_POSTFIX_INCREMENT:
            if (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE) {
                result = type_create(lt->kind, 0, 0, 0);
            } else {
                fprintf(stderr, "typecheck error: cannot postfix increment type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_POSTFIX_DECREMENT:
            if (lt->kind == TYPE_INTEGER || lt->kind == TYPE_DOUBLE) {
                result = type_create(lt->kind, 0, 0, 0);
            } else {
                fprintf(stderr, "typecheck error: cannot postfix decrement type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
            break;
        case EXPR_SUBSCRIPT:
            if (!(lt->kind == TYPE_CARRAY || lt->kind == TYPE_ARRAY)) {
                fprintf(stderr, "typecheck error: cannot index into type %s (", ttos(lt));
                expr_print(e->left, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (rt->kind != TYPE_INTEGER) {
                fprintf(stderr, "typecheck error: cannot index array with type %s (", ttos(rt));
                expr_print(e->right, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            result = type_copy(lt->subtype);
            break;
        case EXPR_CALL:
            /* AUTO CAN BE INFERRED HERE */
            // Walk the list of params and args, typecheck each one.
            struct param_list* param = e->left->symbol->type->params;
            struct expr* arg = e->right;
            struct type* t;

            while (param && arg) {
                t = expr_typecheck(arg);
                if (!type_equals(t, param->type)) {
                    fprintf(stderr, "typecheck error: argument ");
                    expr_print(arg, stderr);
                    fprintf(stderr, "of type ");
                    type_print(t, stderr);
                    fprintf(stderr, "does not match prototype of function %s, which is expecting ", e->left->name);
                    type_print(param->type, stderr);
                    fprintf(stderr, "\n");
                    set_error(true);
                }
                type_delete(t);
                param = param->next;
                arg = arg->right;
            }

            if (param || arg) {
                fprintf(stderr, "typecheck error: call to function %s does not have the correct number of arguments\n", e->left->name);
                set_error(true);
            }
            result = type_copy(lt->subtype);
            break;
        case EXPR_ARG:
            /* Note: No functions with functions as arguments, though this is syntactically possible. */
            result = type_copy(lt);
            break;
        case EXPR_ARRAY_ELEM:
            /* Note: This expression may only appear as part of a global array initialization. It could be part of either a CARRAY or ARRAY declaration; here, the epxression is always given TYPE_ARRAY, since this function doesn't know what kind of declaration it is actaully part of. In decl_typcheck, if it finds the initialized value is TYPE_ARRAY, it will not throw an error if the true type is CARRAY. */
            /* Note: Arrays of functions are handled in decl_typcheck and do not need to be considered here. */
            /* Note: This expr type corresponds to a regular array element (which is not itself an array). This element and the one on its right must be the same. */

            if (!rt) {
                // Base case.
                result = type_create(TYPE_ARRAY, type_copy(lt), 0, expr_create_integer_literal(1));
            } else {
                if (!type_equals(lt, rt->subtype)) {
                    fprintf(stderr, "typecheck error: array initialization cannot contain elements of multiple types. Found %s (", ttos(lt));
                    expr_print(e->left, stderr);
                    fprintf(stderr, ") and %s (", ttos(rt->subtype));
                    expr_print(e->right->left, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                if (e->left->kind < EXPR_NAME) {
                    fprintf(stderr, "typecheck error: array cannot have non-constant initilizer. Found (");
                    expr_print(e->left, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                result = type_copy(rt);
                type_delete(result->subtype);
                result->subtype = type_copy(lt);
                result->array_size->int_literal++;
            }
            break;
        case EXPR_ARRAY_INIT:
            if (!rt) {
                // Base case.
                result = type_create(TYPE_ARRAY, type_copy(lt), 0, expr_create_integer_literal(1));
            } else {
                if (!type_equals(lt, rt->subtype)) {
                    fprintf(stderr, "typecheck error: array initialization cannot contain sub-arrays of differing dimensionality or subtypes. Found %s (", ttos(lt));
                    expr_print(e->left, stderr);
                    fprintf(stderr, ") and %s (", ttos(rt->subtype));
                    expr_print(e->right, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                result = type_copy(rt);
                result->array_size->int_literal++;
            }
            break;
        case EXPR_NAME:
            result = type_copy(e->symbol->type);
            break;
        case EXPR_INT:
            result = type_create(TYPE_INTEGER, 0, 0, 0);
            break;
        case EXPR_DOUBLE:
            result = type_create(TYPE_DOUBLE, 0, 0, 0);
            break;
        case EXPR_BOOLEAN:
            result = type_create(TYPE_BOOLEAN, 0, 0, 0);
            break;
        case EXPR_CHAR:
            result = type_create(TYPE_CHARACTER, 0, 0, 0);
            break;
        case EXPR_STRING:
            result = type_create(TYPE_STRING, 0, 0, 0);
            break;
    }

    type_delete(lt);
    type_delete(rt);

    return result;
}