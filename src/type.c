/* type.c */

#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

struct type* type_create(type_t kind, struct type* subtype, struct param_list* params, struct expr* array_size) {
    struct type* t = calloc(1, sizeof(*t));

    t->kind = kind;
    t->params = params;
    t->subtype = subtype;
    t->array_size = array_size;

    return t;
}

void type_print(struct type* t, FILE* fp) {
    if (!t) {
        return;
    }

    switch (t->kind) {
        case TYPE_VOID:
            fprintf(fp, "void");
            break;
        case TYPE_AUTO:
            // Must infer syntax from t properties
            fprintf(fp, "auto");
            if (t->array_size) {
                // This must be an array
                fprintf(fp, " [");
                expr_print(t->array_size, fp);
                fprintf(fp, "] ");
                type_print(t->subtype, fp);
            }
            break;
        case TYPE_BOOLEAN:
            fprintf(fp, "boolean");
            break;
        case TYPE_CHARACTER:
            fprintf(fp, "char");
            break;
        case TYPE_INTEGER:
            fprintf(fp, "integer");
            break;
        case TYPE_DOUBLE:
            fprintf(fp, "double");
            break;
        case TYPE_STRING:
            fprintf(fp, "string");
            break;
        case TYPE_ARRAY:
            fprintf(fp, "array [");
            expr_print(t->array_size, fp);
            fprintf(fp, "] ");
            type_print(t->subtype, fp);
            break;
        case TYPE_CARRAY:
            fprintf(fp, "carray [");
            expr_print(t->array_size, fp);
            fprintf(fp, "] ");
            type_print(t->subtype, fp);
            break;
        case TYPE_FUNCTION:
            fprintf(fp, "function ");
            type_print(t->subtype, fp);
            fprintf(fp, "(");
            param_list_print(t->params, fp);
            fprintf(fp, ")");
            break;
    }
}

bool type_equals(struct type* a, struct type* b) {
    if (!a && !b) {
        return true;
    } else if (!a || !b) {
        return false;
    }

    // This disgusting IF is checking if the types are the same. It considers CARRAY and ARRAY to be "the same".
    if (a->kind == b->kind || (a->kind == TYPE_ARRAY && b->kind == TYPE_CARRAY) || (b->kind == TYPE_ARRAY && a->kind == TYPE_CARRAY)) {
        if (a->kind < TYPE_ARRAY && b->kind < TYPE_ARRAY) {
            /* Case: A and B both atomic, so types match */
            return true;
        } else if (a->kind < TYPE_FUNCTION && b->kind < TYPE_FUNCTION) {
            /* Case: A and B both arrays, so check that subtypes match */
            if (type_equals(a->subtype, b->subtype)) {
                if (a->array_size && b->array_size && a->array_size->kind == EXPR_INT && b->array_size->kind == EXPR_INT) {
                    // Arrays both have constant length; can check lengths equal at compile time.
                    return a->array_size->int_literal == b->array_size->int_literal;
                } else {
                    // Cannot determine if arrays are the same size at compile time, so just return true and punt to runtime.
                    return true;
                }
            } else {
                return false;
            }

        } else if (a->kind == TYPE_FUNCTION && b->kind == TYPE_FUNCTION) {
            /* Case: A and B both functions, so check that subtypes and params are recursively equal */
            if (!type_equals(a->subtype, b->subtype)) return false;
            struct param_list* a_param = a->params;
            struct param_list* b_param = b->params;
            while (a_param && b_param) {
                if (!type_equals(a_param->type, b_param->type)) return false;
                a_param = a_param->next;
                b_param = b_param->next;
            }

            return a_param == b_param ? true : false;
        }
    } else {
        return false;
    }
    return false;
}

struct type* type_copy(struct type* t) {
    if (!t) return NULL;

    struct type* t_subtype_copy = type_copy(t->subtype);

    // Iteratively deep copy t's param list by walking the linked list if it exists. Note that the symbol pointers for each param are *reused*, and the array_size expr struct is *reused*.
    struct param_list* params_copy_head = NULL;
    if (t->params) {
        params_copy_head = param_list_create(strdup(t->params->name), type_copy(t->params->type), NULL);
        params_copy_head->symbol = t->params->symbol;

        struct param_list* params_copy_curr = params_copy_head;
        struct param_list* params_curr = t->params->next;
        while (params_curr) {
            params_copy_curr->next = param_list_create(strdup(params_curr->name), type_copy(params_curr->type), NULL);
            params_copy_curr->symbol = params_curr->symbol;

            params_copy_curr = params_copy_curr->next;
            params_curr = params_curr->next;
        }
    }

    struct expr* t_array_size_copy = expr_copy(t->array_size);
    return type_create(t->kind, t_subtype_copy, params_copy_head, t_array_size_copy);
}

void type_delete(struct type* t) {
    if (!t) return;

    type_delete(t->subtype);
    if (t->params) {
        struct param_list* params_curr = t->params;
        struct param_list* params_next = params_curr->next;
        while (params_next) {
            param_list_delete(params_curr);
            params_curr = params_next;
            params_next = params_next->next;
        }
    }
    expr_delete(t->array_size);
    free(t);
}

const char* ttos(struct type* t) {
    switch (t->kind) {
        case TYPE_VOID:
            return "void";
        case TYPE_AUTO:
            return "auto";
        case TYPE_BOOLEAN:
            return "boolean";
        case TYPE_CHARACTER:
            return "character";
        case TYPE_INTEGER:
            return "integer";
        case TYPE_DOUBLE:
            return "double";
        case TYPE_STRING:
            return "string";
        case TYPE_ARRAY:
            return "array";
        case TYPE_CARRAY:
            return "carray";
        case TYPE_FUNCTION:
            return "function";
        default:
            return "";
    }
}