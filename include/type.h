#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>

#include "param_list.h"

typedef enum {
    TYPE_VOID,
    TYPE_AUTO,
    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_INTEGER,
    TYPE_DOUBLE,
    TYPE_STRING,

    TYPE_ARRAY,
    TYPE_CARRAY,
    TYPE_FUNCTION,
} type_t;

struct type {
    type_t kind;
    struct param_list* params;
    struct type* subtype;
    struct expr* array_size;
};

struct type* type_create(type_t kind, struct type* subtype, struct param_list* params, struct expr* array_size);

void type_print(struct type* t, FILE* fp);

/**
 * Return a duplicate copy of t, making sure to duplicate subtype and params recursively.
 */
struct type* type_copy(struct type* t);

bool type_equals(struct type* a, struct type* b);

/**
 * Free all the elements of t recursively.
 */
void type_delete(struct type* t);

/**
 * Type TO String: Returns a string corresponding to the kind of a type `t`. For example, if t->kind == TYPE_INTEGER, returns "integer".
 */
const char* ttos(struct type* t);

#endif
