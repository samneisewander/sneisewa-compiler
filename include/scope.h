/**
 * scope.h
 *
 * This module keeps track of a linked list of hash tables, each one
 * representing nested scopes in the program.
 * */

#ifndef SCOPE_H
#define SCOPE_H

#include <stdio.h>
#include <stdlib.h>

#include "symbol.h"

typedef enum {
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_STMT_BLOCK
} scope_t;

/**
 * Causes a new hash table to be pushed on the top of the stack, representing a new scope.
 */
void scope_enter(scope_t scope_kind);

/**
 * Pop the current scope from the symbol table stack.
 *  */
void scope_exit();

/**
 * Insert into the current scope an entry binding a name (identifier) to a symbol object, and setting its "which" property, indicating its position in relative to the frame pointer.
 *
 * Before calling this method for the first time, the user is expected to initialize the symbol table stack by entering a scope calling scope_enter(SCOPE_GLOBAL).
 *
 * This method is NOT responsible for handling illegal duplicate definitions of a name in the same scope. The caller should unsure that the name and symbol may be legally bound BEFORE calling.
 * */
void scope_bind(const char* name, struct symbol* s);

/**
 * Searches the stack of hash tables from top to bottom, looking for the first entry that matches name exactly. If no match is found, it returns null.
 */
struct symbol* scope_lookup(const char* name);

/**
 * Works like scope_lookup except that it only searches the topmost table. This is used to determine whether a symbol has already been defined in the current scope.
 */
struct symbol* scope_lookup_current(const char* name);

/**
 * Returns the depth of the symbol table stack.
 */
int scope_level();

/**
 * Sets a global flag that determines whether name resolutions are printed to stdout or not. Default is false. This piece of global state is used in scope_lookup.
 */
void set_print_resolutions(bool b);

#endif