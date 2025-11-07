
#ifndef DECL_H
#define DECL_H

#include <stdio.h>

#include "expr.h"
#include "stmt.h"
#include "type.h"

/**
 * The Abstract Syntax Tree node kind for a declaration
 */
struct decl {
    char* name;
    struct type* type;
    struct expr* value;
    struct stmt* code;
    struct symbol* symbol;
    struct decl* next;
};

/**
 * A factory function for the Abstract Syntax Tree node kind for a declaration
 *
 * @param name identifier for this decl
 * @param type b-minor type for this decl (i.e. boolean, string, function prototype, etc.)
 * @param value if decl is an expression, the value of the expression
 * @param code if decl is a function, the body of the function
 * @param next next node in decl list. null if end of list
 */
struct decl* decl_create(char* name, struct type* type, struct expr* value, struct stmt* code, struct decl* next);

/**
 * Pretty prints a decl to stdout.
 * @param d decl to print. If d is NULL, does nothing.
 * @param indent numer of tabs to indent
 */
void decl_print(struct decl* d, int indent);

/**
 * @brief Performs name resolution on a declaration.
 *
 * Creates a new symbol, binding it to the name of the declaration in the current scope. If the declaration represents an expression (d->value is not null) then the expression is resolved. If the declaration represents a function (d->code is not null), creates a new scope and resolves the parameters and code.
 * @param d decl on which to perform name resolution.
 */
void decl_resolve(struct decl* d);

#endif
