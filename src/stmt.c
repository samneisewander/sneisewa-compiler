/* stmt.c */

#include "stmt.h"

#include <stdio.h>
#include <stdlib.h>

struct stmt* stmt_create(stmt_t kind, struct decl* decl, struct expr* init_expr, struct expr* expr, struct expr* next_expr, struct stmt* body, struct stmt* else_body, struct stmt* next) {
    struct stmt* s = malloc(sizeof(*s));

    s->kind = kind;
    s->decl = decl;
    s->init_expr = init_expr;
    s->expr = expr;
    s->next_expr = next_expr;
    s->body = body;
    s->else_body = else_body;
    s->next = next;

    return s;
}

void stmt_print(struct stmt* s, int indent) {
    if (!s) {
        return;
    }

    switch (s->kind) {
        case STMT_DECL:
            decl_print(s->decl, indent);
            break;
        case STMT_EXPR:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            expr_print(s->expr);
            printf(";");
            break;
        case STMT_IF_ELSE:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("if (");
            expr_print(s->expr);
            printf(")\n");

            if (s->body->kind != STMT_BLOCK) {
                for (int i = 0; i < indent; i++) putc('\t', stdout);
                printf("{\n");

                stmt_print(s->body, indent + 1);

                printf("\n");
                for (int i = 0; i < indent; i++) putc('\t', stdout);
                printf("}");
            } else {
                stmt_print(s->body, indent);
            }

            if (s->else_body) {
                printf("\n");
                for (int i = 0; i < indent; i++) putc('\t', stdout);
                printf("else ");

                if (s->else_body->kind == STMT_IF_ELSE) {
                    stmt_print(s->else_body, indent);
                } else if (s->else_body->kind != STMT_BLOCK) {
                    printf("\n");
                    for (int i = 0; i < indent; i++) putc('\t', stdout);
                    printf("{\n");

                    stmt_print(s->else_body, indent + 1);

                    printf("\n");
                    for (int i = 0; i < indent; i++) putc('\t', stdout);
                    printf("}");
                } else {
                    printf("\n");
                    stmt_print(s->else_body, indent);
                }
            }
            break;
        case STMT_FOR:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("for (");
            expr_print(s->init_expr);
            putc(';', stdout);
            expr_print(s->expr);
            putc(';', stdout);
            expr_print(s->expr);
            printf(")\n");

            if (s->body->kind != STMT_BLOCK) {
                for (int i = 0; i < indent; i++) putc('\t', stdout);
                printf("{\n");

                stmt_print(s->body, indent + 1);

                printf("\n");
                for (int i = 0; i < indent; i++) putc('\t', stdout);
                printf("}");
            } else {
                stmt_print(s->body, indent);
            }

            break;
        case STMT_PRINT:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("print ");
            expr_print(s->expr);
            printf(";");
            break;
        case STMT_RETURN:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("return ");
            expr_print(s->expr);
            putc(';', stdout);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("{\n");

            stmt_print(s->body, indent + 1);

            putc('\n', stdout);
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("}");
            break;
    }

    if (s->next) {
        printf("\n");
        stmt_print(s->next, indent);
    }
}
