/* stmt.c */

#include "stmt.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "decl.h"
#include "error.h"
#include "scope.h"

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
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
            decl_print(s->decl, indent);
            break;
        case STMT_EXPR:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            expr_print(s->expr, stdout);
            printf(";");
            break;
        case STMT_IF_ELSE:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("if (");
            expr_print(s->expr, stdout);
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
            expr_print(s->init_expr, stdout);
            putc(';', stdout);
            expr_print(s->expr, stdout);
            putc(';', stdout);
            expr_print(s->expr, stdout);
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
            expr_print(s->expr, stdout);
            printf(";");
            break;
        case STMT_RETURN:
            for (int i = 0; i < indent; i++) putc('\t', stdout);
            printf("return ");
            expr_print(s->expr, stdout);
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

void stmt_resolve(struct stmt* s) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
            decl_resolve(s->decl);
            break;
        case STMT_EXPR:
            expr_resolve(s->expr);
            break;
        case STMT_IF_ELSE:
            expr_resolve(s->expr);
            if (s->body->kind == STMT_DECL) {
                fprintf(stderr, "resolver: cannot declare %s in single-line if statement.\n", s->body->decl->name);
                set_error(true);
                break;
            }
            stmt_resolve(s->body);
            if (s->else_body) {
                stmt_resolve(s->else_body);
            }
            break;
        case STMT_FOR:
            expr_resolve(s->init_expr);
            expr_resolve(s->expr);
            expr_resolve(s->next_expr);
            stmt_resolve(s->body);
            break;
        case STMT_PRINT:
        case STMT_RETURN:
            expr_resolve(s->expr);
            break;
        case STMT_BLOCK:
            scope_enter(SCOPE_STMT_BLOCK);
            stmt_resolve(s->body);
            scope_exit();
            break;
    }

    stmt_resolve(s->next);
}

void stmt_typecheck(struct stmt* s, struct symbol* prototype) {
    if (!s) return;

    struct type* t;
    struct expr* e;
    switch (s->kind) {
        case STMT_DECL:
            decl_typecheck(s->decl);
            break;
        case STMT_EXPR:
            type_delete(expr_typecheck(s->expr));
            break;
        case STMT_IF_ELSE:
            t = expr_typecheck(s->expr);
            if (t->kind != TYPE_BOOLEAN) {
                fprintf(stderr, "typecheck error: control expression of if statement cannot be of type %s (", ttos(t));
                expr_print(s->expr, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            type_delete(t);
            stmt_typecheck(s->body, prototype);
            stmt_typecheck(s->else_body, prototype);
            break;
        case STMT_FOR:
            type_delete(expr_typecheck(s->init_expr));
            t = expr_typecheck(s->expr);
            if (t->kind != TYPE_BOOLEAN) {
                fprintf(stderr, "typecheck error: control expression of for loop cannot be of type %s (", ttos(t));
                expr_print(s->expr, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            type_delete(t);
            type_delete(expr_typecheck(s->next_expr));
            stmt_typecheck(s->body, prototype);
            break;
        case STMT_PRINT:
            /* Can print anything except auto, void, or function  */

            // Walk the contents of s->expr, which should be a linked list of EXPR_ARGs, and check that they are all printable.
            e = s->expr;
            while (e) {
                t = expr_typecheck(e->left);
                if (t->kind == TYPE_AUTO || t->kind == TYPE_VOID || t->kind == TYPE_FUNCTION) {
                    fprintf(stderr, "typecheck error: cannot print type %s (", ttos(t));
                    expr_print(s->expr, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }

                type_delete(t);
                e = e->right;
            }
            break;
        case STMT_RETURN:
            /* Return type must match function prototype. */
            t = expr_typecheck(s->expr);
            if (!t) {
                t = type_create(TYPE_VOID, 0, 0, 0);
            }
            if (!type_equals(t, prototype->type->subtype)) {
                fprintf(stderr, "typecheck error: return type of function %s is %s, instead got type %s (", prototype->name, ttos(prototype->type->subtype), ttos(t));
                expr_print(s->expr, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
            switch (t->kind) {
                case TYPE_ARRAY:
                case TYPE_CARRAY:
                case TYPE_FUNCTION:
                    fprintf(stderr, "typecheck error: cannot return type %s from function (%s)\n", ttos(t), prototype->name);
                    set_error(true);
                    break;
                default:
                    // Shoo, gcc warnings!
                    break;
            }
            type_delete(t);
            break;
        case STMT_BLOCK:
            stmt_typecheck(s->body, prototype);
            break;
            /* more cases here */
    }

    stmt_typecheck(s->next, prototype);
}