/* stmt.c */

#include "stmt.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "decl.h"
#include "error.h"
#include "label.h"
#include "scope.h"
#include "scratch.h"

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

void stmt_resolve(struct stmt* s, struct symbol* prototype) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
            decl_resolve(s->decl, prototype);
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
            stmt_resolve(s->body, prototype);
            if (s->else_body) {
                stmt_resolve(s->else_body, prototype);
            }
            break;
        case STMT_FOR:
            expr_resolve(s->init_expr);
            expr_resolve(s->expr);
            expr_resolve(s->next_expr);
            stmt_resolve(s->body, prototype);
            break;
        case STMT_PRINT:
        case STMT_RETURN:
            expr_resolve(s->expr);
            break;
        case STMT_BLOCK:
            scope_enter(SCOPE_STMT_BLOCK);
            stmt_resolve(s->body, prototype);
            scope_exit();
            break;
    }

    stmt_resolve(s->next, prototype);
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
            if (s->init_expr) {
                type_delete(expr_typecheck(s->init_expr));
            }
            if (s->expr) {
                t = expr_typecheck(s->expr);
                if (t->kind != TYPE_BOOLEAN) {
                    fprintf(stderr, "typecheck error: control expression of for loop cannot be of type %s (", ttos(t));
                    expr_print(s->expr, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                type_delete(t);
            }
            if (s->next_expr) {
                type_delete(expr_typecheck(s->next_expr));
            }
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

            /* Prevent returning auto if function return type is auto. Otherwise, infer return value type from function prototype. */
            if (prototype->type->subtype->kind == TYPE_AUTO) {
                if (t->kind == TYPE_AUTO) {
                    fprintf(stderr, "typecheck error: cannot return type auto (");
                    expr_print(s->expr, stderr);
                    fprintf(stderr, ") from automatically typed function (%s)\n", prototype->name);
                    set_error(true);
                    break;
                }
                /* Infer function return type from return type. */
                type_delete(prototype->type->subtype);
                prototype->type->subtype = type_copy(t);
            }

            switch (t->kind) {
                case TYPE_ARRAY:
                case TYPE_CARRAY:
                case TYPE_FUNCTION:
                    fprintf(stderr, "typecheck error: cannot return type %s from function (%s)\n", ttos(t), prototype->name);
                    set_error(true);
                    break;
                case TYPE_AUTO:
                    // Infer return expression type from function prototype
                    type_delete(t);
                    t = type_copy(prototype->type->subtype);
                    break;
                default:
                    // Shoo, gcc warnings!
                    break;
            }

            if (!type_equals(t, prototype->type->subtype)) {
                fprintf(stderr, "typecheck error: return type of function %s is %s, instead got type %s (", prototype->name, ttos(prototype->type->subtype), ttos(t));
                expr_print(s->expr, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
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

void stmt_codegen(struct stmt* s, struct symbol* prototype, FILE* outfile) {
    if (!s) return;
    int label_1, label_2;
    struct expr* arg;

    switch (s->kind) {
        case STMT_EXPR:
            expr_codegen(s->expr, outfile);
            scratch_free(s->expr->reg);
            break;
        case STMT_DECL:
            decl_codegen(s->decl, outfile);
            break;
        case STMT_IF_ELSE:
            expr_codegen(s->expr, outfile);
            fprintf(outfile, "CMP $0, %s\n", scratch_name(s->expr->reg));
            scratch_free(s->expr->reg);

            // Create false_label and done_label now, only store first label number, just use math to get the second one.
            label_1 = label_create();
            label_2 = label_create();

            fprintf(outfile, "JE %s\n", label_name(label_1));
            stmt_codegen(s->body, prototype, outfile);  // True statements
            fprintf(outfile, "JMP %s\n", label_name(label_2));
            fprintf(outfile, "%s:\n", label_name(label_1));  // False label
            stmt_codegen(s->else_body, prototype, outfile);  // False statements
            fprintf(outfile, "%s:\n", label_name(label_2));  // Done label
            break;
        case STMT_FOR:
            if (s->init_expr) {
                expr_codegen(s->init_expr, outfile);
                scratch_free(s->init_expr->reg);
            }

            // Create top_label and done_label now, storing the first label number and using math to get the second one.
            label_1 = label_create();  // top_label
            label_2 = label_create();  // done_label

            fprintf(outfile, "%s:\n", label_name(label_1));  // Print top_label
            if (s->expr) {
                expr_codegen(s->expr, outfile);
                fprintf(outfile, "CMP $0, %s\n", scratch_name(s->expr->reg));
                fprintf(outfile, "JE %s\n", label_name(label_2));
                scratch_free(s->expr->reg);
            }
            stmt_codegen(s->body, prototype, outfile);
            if (s->next_expr) {
                expr_codegen(s->next_expr, outfile);
                scratch_free(s->next_expr->reg);
            }
            fprintf(outfile, "JMP %s\n", label_name(label_1));
            fprintf(outfile, "%s:\n", label_name(label_2));  // Print done_label
            break;
        case STMT_PRINT:
            arg = s->expr;
            // Save old argument registers (%rdi, %rsi, %rdx, %rcx, %r8, and %r9, in that order.)
            fprintf(outfile, "# *** Print START ***\n");
            fprintf(outfile, "PUSH %%rdi\n");
            fprintf(outfile, "PUSH %%rsi\n");
            fprintf(outfile, "PUSH %%rdx\n");
            fprintf(outfile, "PUSH %%rcx\n");
            fprintf(outfile, "PUSH %%r8\n");
            fprintf(outfile, "PUSH %%r9\n");
            while (arg) {
                // Write arguments into argument registers, exit if there are more than 6
                expr_codegen(arg->left, outfile);
                fprintf(outfile, "MOVQ %s, %%rdi\n", scratch_name(arg->left->reg));
                scratch_free(arg->left->reg);

                // Peform function call
                switch (expr_typecheck(arg->left)->kind) {
                    case TYPE_INTEGER:
                        fprintf(outfile, "CALL %s\n", "print_integer");
                        break;
                    case TYPE_BOOLEAN:
                        fprintf(outfile, "CALL %s\n", "print_boolean");
                        break;
                    case TYPE_CHARACTER:
                        fprintf(outfile, "CALL %s\n", "print_character");
                        break;
                    case TYPE_STRING:
                        fprintf(outfile, "CALL %s\n", "print_string");
                        break;
                    default:
                        break;
                }

                arg = arg->right;
            }
            // Restore old argument registers
            fprintf(outfile, "POP %%r9\n");
            fprintf(outfile, "POP %%r8\n");
            fprintf(outfile, "POP %%rcx\n");
            fprintf(outfile, "POP %%rdx\n");
            fprintf(outfile, "POP %%rsi\n");
            fprintf(outfile, "POP %%rdi\n");
            fprintf(outfile, "# *** Print END ***\n");
            break;
        case STMT_RETURN:
            expr_codegen(s->expr, outfile);
            fprintf(outfile, "MOVQ %s, %%rax\n", scratch_name(s->expr->reg));
            fprintf(outfile, "JMP %s_epilogue\n", prototype->name);
            scratch_free(s->expr->reg);
            break;
        case STMT_BLOCK:
            stmt_codegen(s->body, prototype, outfile);
    }
    stmt_codegen(s->next, prototype, outfile);
}