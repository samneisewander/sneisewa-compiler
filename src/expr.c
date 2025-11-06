/* expr.c */

#include "expr.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"

struct expr* expr_create(expr_t kind, struct expr* left, struct expr* right) {
    struct expr* e = calloc(1, sizeof(*e));
    e->kind = kind;
    e->left = left;
    e->right = right;
    return e;
}

struct expr* expr_create_name(const char* n) {
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

struct expr* expr_create_string_literal(const char* str) {
    struct expr* e = calloc(1, sizeof(*e));
    e->string_literal = str;
    e->kind = EXPR_STRING;
    return e;
}

// Pretty prints expr e, wrapped in parentheses
void expr_print_parens(struct expr* e) {
    putc('(', stdout);
    expr_print(e);
    putc(')', stdout);
}

// Pretty prints an expr, removing unnecessary whitespace or parentheses while preserving the structure of the AST. If expr is null, does nothing.
void expr_print(struct expr* e) {
    if (!e) {
        return;
    }

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
            expr_print(e->left);
            printf("=");
            expr_print(e->right);
            break;
        case EXPR_LOGICAL_OR:
            if (e->left->kind < EXPR_LOGICAL_OR) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("||");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LOGICAL_OR) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_LOGICAL_AND:
            if (e->left->kind < EXPR_LOGICAL_AND) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("&&");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LOGICAL_AND) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_LESS_THAN:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("<");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LESS_THAN) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_LESS_THAN_OR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("<=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_LESS_THAN_OR_EQUAL) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("==");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_EQUAL) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_GREATER_THAN:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf(">");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_GREATER_THAN) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_GREATER_THAN_OR_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf(">=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_GREATER_THAN_OR_EQUAL) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_NOT_EQUAL:
            if (e->left->kind < EXPR_LESS_THAN) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("!=");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_NOT_EQUAL) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_ADD:
            if (e->left->kind < EXPR_ADD) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("+");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_ADD) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_SUBTRACT:
            if (e->left->kind < EXPR_ADD) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("-");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_SUBTRACT) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_MULTIPLY:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("*");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_MULTIPLY) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_DIVIDE:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("/");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_DIVIDE) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_REMAINDER:
            if (e->left->kind < EXPR_MULTIPLY) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("%%");
            if (e->right->kind < EXPR_NAME && e->right->kind <= EXPR_REMAINDER) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_EXPONENT:
            if (e->left->kind < EXPR_NAME && e->left->kind <= EXPR_EXPONENT) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("^");
            if (e->right->kind < EXPR_EXPONENT) {
                expr_print_parens(e->right);
            } else {
                expr_print(e->right);
            }
            break;
        case EXPR_UNARY_NEGATION:
        case EXPR_LOGICAL_NOT:
            printf("!");
            if (e->left->kind < EXPR_UNARY_NEGATION) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            break;
        case EXPR_UNARY_ARRAY_LENGTH:
            printf("#");
            if (e->left->kind < EXPR_UNARY_ARRAY_LENGTH) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            break;
        case EXPR_POSTFIX_INCREMENT:
            if (e->left->kind < EXPR_POSTFIX_INCREMENT) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("++");
            break;
        case EXPR_POSTFIX_DECREMENT:
            if (e->left->kind < EXPR_POSTFIX_INCREMENT) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("--");
            break;
        case EXPR_SUBSCRIPT:
            if (e->left->kind < EXPR_SUBSCRIPT) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("[");
            expr_print(e->right);
            printf("]");
            break;
        case EXPR_CALL:
            if (e->left->kind < EXPR_SUBSCRIPT) {
                expr_print_parens(e->left);
            } else {
                expr_print(e->left);
            }
            printf("(");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_ARG:
            if (e->left->kind == EXPR_ARG) {
                putc('{', stdout);
                expr_print(e->left);
                putc('}', stdout);
            } else {
                expr_print(e->left);
            }
            if (e->right) {
                printf(",");
                expr_print(e->right);
            }
            break;
        case EXPR_ARRAY_INIT:
            putc('{', stdout);
            expr_print(e->left);
            putc('}', stdout);
            break;
        case EXPR_NAME:
            printf("%s", e->name);
            break;
        case EXPR_INT:
            printf("%ld", e->int_literal);
            break;
        case EXPR_DOUBLE:
            printf("%lf", e->double_literal);
            break;
        case EXPR_BOOLEAN:
            if (e->bool_literal) {
                printf("true");
            } else {
                printf("false");
            }
            break;
        case EXPR_CHAR:
            printf("'\\0x%x'", e->char_literal);
            break;
        case EXPR_STRING:
            string_encode(e->string_literal, buf);
            printf("%s", buf);
            break;
    }
}