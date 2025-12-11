/* expr.c */

#include "expr.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoder.h"
#include "error.h"
#include "label.h"
#include "scope.h"
#include "scratch.h"
#include "symbol.h"

/**
 * - Cannot print main
 * - Cannot print function with return type auto
 * - Cannot print auto
 * -
 */

const char* arg_reg_map[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

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

    struct param_list* param;
    struct expr* arg;
    struct type* arg_t;

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
            } else if (lt->kind == TYPE_VOID || lt->kind == TYPE_ARRAY || lt->kind == TYPE_CARRAY || lt->kind == TYPE_FUNCTION) {
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
            if (type_equals(lt, rt) && lt->kind == TYPE_INTEGER) {
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
            if (lt->kind == TYPE_ARRAY) {
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
            param = e->left->symbol->type->params;
            arg = e->right;

            while (param && arg) {
                arg_t = expr_typecheck(arg->left);
                if (!type_equals(arg_t, param->type)) {
                    fprintf(stderr, "typecheck error: argument ");
                    expr_print(arg, stderr);
                    fprintf(stderr, "of type ");
                    type_print(arg_t, stderr);
                    fprintf(stderr, "does not match prototype of function %s, which is expecting ", e->left->name);
                    type_print(param->type, stderr);
                    fprintf(stderr, "\n");
                    set_error(true);
                }
                type_delete(arg_t);
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
            if (e->symbol) {
                result = type_copy(e->symbol->type);
            } else {
                result = type_create(TYPE_INTEGER, 0, 0, 0);
            }
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

void expr_codegen(struct expr* e, FILE* outfile) {
    if (!e) return;
    char encoded_string[256] = {0};
    const char* label = NULL;
    struct type* type;
    struct expr* arg;
    int reg, reg2, label1, label2;
    int arg_count = 0;

    switch (e->kind) {
        // Interior node: generate children, then add them.
        case EXPR_ASSIGNMENT:
            expr_codegen(e->right, outfile);
            // Handle special case where subscript should return address at offset from memory
            if (e->left->kind == EXPR_SUBSCRIPT) {
                // 1. Evaluate indexing expression
                reg = scratch_alloc();
                expr_codegen(e->left->right, outfile);

                // 2. Perform bounds checking if necessary
                if (e->left->left->symbol->type->kind == TYPE_ARRAY) {
                    // 2a. Load first element of array (the size) into a register
                    reg2 = scratch_alloc();
                    fprintf(outfile, "MOVQ (%s), %s\n", symbol_codegen(e->left->left->symbol), scratch_name(reg2));

                    // 2b. Check if index is greater than array size or less than zero
                    label1 = label_create();
                    label2 = label_create();
                    /**
                     * CAREFUL: The CMPQ instruction is tricky!
                     * CMPQ %rax, %rbx
                     * JG               # Taken if %rbx > %rax
                     */
                    fprintf(outfile, "CMPQ %s, %s\n", scratch_name(reg2), scratch_name(e->left->right->reg));
                    fprintf(outfile, "JGE %s\n", label_name(label1));
                    fprintf(outfile, "XOR %s, %s\n", scratch_name(reg2), scratch_name(reg2));
                    fprintf(outfile, "CMPQ %s, %s\n", scratch_name(reg2), scratch_name(e->left->right->reg));
                    fprintf(outfile, "JL %s\n", label_name(label1));
                    scratch_free(reg2);
                    fprintf(outfile, "JMP %s\n", label_name(label2));
                    // 2c. Call exit(EXIT_FAILURE)
                    fprintf(outfile, "%s:\n", label_name(label1));
                    fprintf(outfile, "MOVQ $1, %%rdi\n");
                    fprintf(outfile, "MOVQ $60, %%rax\n");
                    fprintf(outfile, "SYSCALL\n");
                    // 2d. Otherwise, add 1 to index to skip over array size
                    fprintf(outfile, "%s:\n", label_name(label2));
                    fprintf(outfile, "ADDQ $1, %s\n", scratch_name(e->left->right->reg));
                }

                // 3. Load the address of the first array element
                fprintf(outfile, "LEAQ %s, %s\n", symbol_codegen(e->left->left->symbol), scratch_name(reg));

                // 4. Write the new value.
                fprintf(outfile, "MOVQ %s, (%s, %s, 8)\n",
                        scratch_name(e->right->reg),         // Source
                        scratch_name(reg),                   // Destination base
                        scratch_name(e->left->right->reg));  // Destination offset

                // 5. Free registers.
                scratch_free(e->left->right->reg);
                scratch_free(reg);
            } else {
                fprintf(outfile, "MOVQ %s, %s\n", scratch_name(e->right->reg), symbol_codegen(e->left->symbol));
                e->reg = e->right->reg;
            }
            break;
        case EXPR_LOGICAL_OR:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ $1, %s\n", scratch_name(e->left->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "CMPQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));

            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));

            fprintf(outfile, "%s:\n", label_name(label2));
            e->reg = e->right->reg;
            scratch_free(e->left->reg);

            break;
        case EXPR_LOGICAL_AND:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ $0, %s\n", scratch_name(e->left->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "CMPQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));

            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));

            fprintf(outfile, "%s:\n", label_name(label2));
            e->reg = e->right->reg;
            scratch_free(e->left->reg);

            break;
        case EXPR_LESS_THAN:
            /**
             * CAREFUL: The CMPQ instruction is tricky!
             * CMPQ %rax, %rbx
             * JG               # Taken if %rbx > %rax
             */
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JLE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));

            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_LESS_THAN_OR_EQUAL:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JLE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));

            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_EQUAL:
            // Not going to support equality between strings or arrays.
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();
            type = expr_typecheck(e->left);

            // Guard stuff not implemented
            switch (type->kind) {
                case TYPE_ARRAY:
                case TYPE_CARRAY:
                case TYPE_STRING:
                    fprintf(stderr, "codegen: comparison between %ss not implemented\n", ttos(type));
                default:
                    break;
            }

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));  // Equal label
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));  // done_label

            e->reg = e->right->reg;
            scratch_free(e->left->reg);

            break;
        case EXPR_GREATER_THAN:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JG %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));

            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_GREATER_THAN_OR_EQUAL:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JGE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));

            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_NOT_EQUAL:
            // Not going to support equality between strings or arrays.
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            label1 = label_create();
            label2 = label_create();
            type = expr_typecheck(e->left);

            // Guard stuff not implemented
            switch (type->kind) {
                case TYPE_ARRAY:
                case TYPE_CARRAY:
                case TYPE_STRING:
                    fprintf(stderr, "codegen: comparison between %ss not implemented\n", ttos(type));
                default:
                    break;
            }

            fprintf(outfile, "CMPQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));
            fprintf(outfile, "%s:\n", label_name(label1));  // Equal label
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "%s:\n", label_name(label2));  // done_label

            scratch_free(e->left->reg);
            break;
        case EXPR_ADD:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            fprintf(outfile, "ADDQ %s, %s\n", scratch_name(e->left->reg), scratch_name(e->right->reg));
            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_SUBTRACT:
            /**
             * CAREFUL: The SUBQ instruction is tricky!
             * SUBQ %rax, %rbx # rbx <= rbx - rax
             */
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            fprintf(outfile, "SUBQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
            e->reg = e->left->reg;
            scratch_free(e->right->reg);
            break;
        case EXPR_MULTIPLY:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            fprintf(outfile, "MOVQ %s, %%rax\n", scratch_name(e->left->reg));
            fprintf(outfile, "IMULQ %s\n", scratch_name(e->right->reg));
            fprintf(outfile, "MOVQ %%rax, %s\n", scratch_name(e->right->reg));
            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_DIVIDE:
        case EXPR_REMAINDER:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);
            fprintf(outfile, "MOVQ %s, %%rax\n", scratch_name(e->left->reg));
            fprintf(outfile, "CQO\n");
            fprintf(outfile, "IDIVQ %s\n", scratch_name(e->right->reg));
            if (e->kind == EXPR_DIVIDE) {
                fprintf(outfile, "MOVQ %%rax, %s\n", scratch_name(e->right->reg));
            } else {
                fprintf(outfile, "MOVQ %%rdx, %s\n", scratch_name(e->right->reg));
            }
            e->reg = e->right->reg;
            scratch_free(e->left->reg);
            break;
        case EXPR_EXPONENT:
            expr_codegen(e->left, outfile);
            expr_codegen(e->right, outfile);

            // Library function call
            fprintf(outfile, "PUSH %%rdi\n");
            fprintf(outfile, "PUSH %%rsi\n");
            fprintf(outfile, "PUSH %%rdx\n");
            fprintf(outfile, "PUSH %%rcx\n");
            fprintf(outfile, "PUSH %%r8\n");
            fprintf(outfile, "PUSH %%r9\n");

            // Write arguments into argument registers
            fprintf(outfile, "MOVQ %s, %%rdi\n", scratch_name(e->left->reg));
            fprintf(outfile, "MOVQ %s, %%rsi\n", scratch_name(e->right->reg));

            // Peform function call
            fprintf(outfile, "CALL integer_power\n");
            // Restore old argument registers
            fprintf(outfile, "POP %%r9\n");
            fprintf(outfile, "POP %%r8\n");
            fprintf(outfile, "POP %%rcx\n");
            fprintf(outfile, "POP %%rdx\n");
            fprintf(outfile, "POP %%rsi\n");
            fprintf(outfile, "POP %%rdi\n");

            fprintf(outfile, "MOVQ %%rax, %s\n", scratch_name(e->right->reg));
            scratch_free(e->left->reg);
            e->reg = e->right->reg;
            break;
        case EXPR_UNARY_NEGATION:
            expr_codegen(e->left, outfile);
            fprintf(outfile, "NEGQ %s\n", scratch_name(e->left->reg));
            e->reg = e->left->reg;
            break;
        case EXPR_LOGICAL_NOT:
            expr_codegen(e->left, outfile);
            label1 = label_create();
            label2 = label_create();

            fprintf(outfile, "CMPQ $1, %s\n", scratch_name(e->left->reg));
            fprintf(outfile, "JE %s\n", label_name(label1));
            fprintf(outfile, "MOVQ $1, %s\n", scratch_name(e->left->reg));
            fprintf(outfile, "JMP %s\n", label_name(label2));

            fprintf(outfile, "%s:\n", label_name(label1));
            fprintf(outfile, "MOVQ $0, %s\n", scratch_name(e->left->reg));

            fprintf(outfile, "%s:\n", label_name(label2));
            e->reg = e->left->reg;

            break;
        case EXPR_UNARY_ARRAY_LENGTH:
            expr_codegen(e->left, outfile);
            fprintf(outfile, "MOVQ %s, %s\n", scratch_name(e->left->reg), scratch_name(e->left->reg));
            e->reg = e->left->reg;
            break;
        case EXPR_POSTFIX_INCREMENT:
            expr_codegen(e->left, outfile);
            fprintf(outfile, "INCQ %s\n", scratch_name(e->left->reg));
            e->reg = e->left->reg;
            break;
        case EXPR_POSTFIX_DECREMENT:
            expr_codegen(e->left, outfile);
            fprintf(outfile, "DECQ %s\n", scratch_name(e->left->reg));
            e->reg = e->left->reg;
            break;
        case EXPR_SUBSCRIPT:
            expr_codegen(e->right, outfile);

            if (e->left->symbol->type->kind == TYPE_ARRAY) {
                // 2a. Load first element of array (the size) into a register
                reg2 = scratch_alloc();
                fprintf(outfile, "MOVQ (%s), %s\n", symbol_codegen(e->left->symbol), scratch_name(reg2));

                // 2b. Check if index is greater than array size or less than zero
                label1 = label_create();
                label2 = label_create();
                /**
                 * CAREFUL: The CMPQ instruction is tricky!
                 * CMPQ %rax, %rbx
                 * JG               # Taken if %rbx > %rax
                 */
                fprintf(outfile, "CMPQ %s, %s\n", scratch_name(reg2), scratch_name(e->right->reg));
                fprintf(outfile, "JGE %s\n", label_name(label1));
                fprintf(outfile, "XOR %s, %s\n", scratch_name(reg2), scratch_name(reg2));
                fprintf(outfile, "CMPQ %s, %s\n", scratch_name(reg2), scratch_name(e->right->reg));
                fprintf(outfile, "JL %s\n", label_name(label1));
                scratch_free(reg2);
                fprintf(outfile, "JMP %s\n", label_name(label2));
                // 2c. Call exit(EXIT_FAILURE)
                fprintf(outfile, "%s:\n", label_name(label1));
                fprintf(outfile, "MOVQ $1, %%rdi\n");
                fprintf(outfile, "MOVQ $60, %%rax\n");
                fprintf(outfile, "SYSCALL\n");
                // 2d. Otherwise, add 1 to index to skip over array size
                fprintf(outfile, "%s:\n", label_name(label2));
                fprintf(outfile, "ADDQ $1, %s\n", scratch_name(e->right->reg));
            }

            reg = scratch_alloc();
            fprintf(outfile, "LEAQ %s, %s\n", symbol_codegen(e->left->symbol), scratch_name(reg));
            fprintf(outfile, "MOVQ (%s, %s, 8), %s\n", scratch_name(reg), scratch_name(e->right->reg), scratch_name(e->right->reg));
            e->reg = e->right->reg;
            scratch_free(reg);
            break;
        case EXPR_CALL:
            // Save old argument registers (%rdi, %rsi, %rdx, %rcx, %r8, and %r9, in that order.)
            // Henry saved and restored the argument registers in the prologue and epilogue... I think this is functionally equivalent but will revisit if there are problems.
            fprintf(outfile, "PUSH %%rdi\n");
            fprintf(outfile, "PUSH %%rsi\n");
            fprintf(outfile, "PUSH %%rdx\n");
            fprintf(outfile, "PUSH %%rcx\n");
            fprintf(outfile, "PUSH %%r8\n");
            fprintf(outfile, "PUSH %%r9\n");

            // Write arguments into argument registers, exit if there are more than 6
            arg = e->right;
            while (arg) {
                if (arg_count > 5) {
                    fprintf(stderr, "codegen: function calls with more than 6 arguments not supported.\n");
                    exit(EXIT_FAILURE);
                }
                expr_codegen(arg->left, outfile);
                fprintf(outfile, "MOVQ %s, %s\n", scratch_name(arg->left->reg), arg_reg_map[arg_count]);
                scratch_free(arg->left->reg);
                arg = arg->right;
                arg_count++;
            }

            // Peform function call
            fprintf(outfile, "CALL %s\n", e->left->name);
            reg = scratch_alloc();
            fprintf(outfile, "MOVQ %%rax, %s\n", scratch_name(reg));

            // Restore old argument registers
            fprintf(outfile, "POP %%r9\n");
            fprintf(outfile, "POP %%r8\n");
            fprintf(outfile, "POP %%rcx\n");
            fprintf(outfile, "POP %%rdx\n");
            fprintf(outfile, "POP %%rsi\n");
            fprintf(outfile, "POP %%rdi\n");

            break;
        case EXPR_NAME:
            // What if the name is to an array or string type? Then we probably want to use LEAQ on the output of symbol_codegen, not just move the value at that address (which would give us the first character or array element instead of a pointer to the whole structure).
            e->reg = scratch_alloc();
            fprintf(outfile, "MOVQ %s, %s\n", symbol_codegen(e->symbol), scratch_name(e->reg));
            break;
        case EXPR_INT:
            e->reg = scratch_alloc();
            fprintf(outfile, "MOVQ $%ld, %s\n", e->int_literal, scratch_name(e->reg));
            break;
        case EXPR_DOUBLE:
            fprintf(stderr, "codegen: floating point not supported.\n");
            exit(EXIT_FAILURE);
            break;
        case EXPR_BOOLEAN:
            e->reg = scratch_alloc();
            fprintf(outfile, "MOVQ $%ld, %s\n", e->bool_literal ? 1L : 0L, scratch_name(e->reg));
            break;
        case EXPR_CHAR:
            e->reg = scratch_alloc();
            char_encode(e->char_literal, encoded_string);
            fprintf(outfile, "MOVQ $%s, %s\n", encoded_string, scratch_name(e->reg));
            break;
        case EXPR_STRING:
            e->reg = scratch_alloc();
            label = label_name(label_create());

            string_encode(e->string_literal, encoded_string);
            fprintf(outfile, ".data\n");
            fprintf(outfile, "%s: .string %s\n", label, encoded_string);
            fprintf(outfile, ".text\n");
            fprintf(outfile, "LEAQ %s, %s\n", label, scratch_name(e->reg));
            break;
        default:
            break;
    }
}