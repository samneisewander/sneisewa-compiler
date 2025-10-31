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
    e->string_literal = n;
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

void expr_print(struct expr* e) {
    if (!e) {
        return;
    }

    char buf[BUFSIZ] = {0};

    switch (e->kind) {
        case EXPR_ASSIGNMENT:
            expr_print(e->left);
            printf("=");
            expr_print(e->right);
            break;
        case EXPR_LOGICAL_OR:
            expr_print(e->left);
            printf("||");
            expr_print(e->right);
            break;
        case EXPR_LOGICAL_AND:
            expr_print(e->left);
            printf("&&");
            expr_print(e->right);
            break;
        case EXPR_LESS_THAN:
            expr_print(e->left);
            printf("<");
            expr_print(e->right);
            break;
        case EXPR_LESS_THAN_OR_EQUAL:
            expr_print(e->left);
            printf("<=");
            expr_print(e->right);
            break;
        case EXPR_EQUAL:
            expr_print(e->left);
            printf("==");
            expr_print(e->right);
            break;
        case EXPR_GREATER_THAN:
            expr_print(e->left);
            printf(">");
            expr_print(e->right);
            break;
        case EXPR_GREATER_THAN_OR_EQUAL:
            expr_print(e->left);
            printf(">=");
            expr_print(e->right);
            break;
        case EXPR_NOT_EQUAL:
            expr_print(e->left);
            printf("!=");
            expr_print(e->right);
            break;
        case EXPR_ADD:
            expr_print(e->left);
            printf("+");
            expr_print(e->right);
            break;
        case EXPR_SUBTRACT:
            expr_print(e->left);
            printf("-");
            expr_print(e->right);
            break;
        case EXPR_MULTIPLY:
            expr_print(e->left);
            printf("*");
            expr_print(e->right);
            break;
        case EXPR_DIVIDE:
            expr_print(e->left);
            printf("/");
            expr_print(e->right);
            break;
        case EXPR_REMAINDER:
            expr_print(e->left);
            printf("%%");
            expr_print(e->right);
            break;
        case EXPR_EXPONENT:
            expr_print(e->left);
            printf("^");
            expr_print(e->right);
            break;
        case EXPR_UNARY_NEGATION:
        case EXPR_LOGICAL_NOT:
            printf("!");
            expr_print(e->left);
            break;
        case EXPR_UNARY_ARRAY_LENGTH:
            printf("#");
            expr_print(e->left);
            break;
        case EXPR_POSTFIX_INCREMENT:
            expr_print(e->left);
            printf("++");
            break;
        case EXPR_POSTFIX_DECREMENT:
            expr_print(e->left);
            printf("--");
            break;
        case EXPR_SUBSCRIPT:
            expr_print(e->left);
            printf("[");
            expr_print(e->right);
            printf("]");
            break;
        case EXPR_CALL:
            expr_print(e->left);
            printf("(");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_ARG:
            expr_print(e->left);
            if (e->right) {
                printf(",");
                expr_print(e->right);
            }
            break;
        case EXPR_NAME:
            printf("%s", e->string_literal);
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
            printf("'\\%x'", e->char_literal);
            break;
        case EXPR_STRING:
            string_encode(e->string_literal, buf);
            printf("%s", buf);
            break;
    }
}