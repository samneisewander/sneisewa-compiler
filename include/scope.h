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

/* PUBLIC METHODS */

/**
 * Causes a new hash table to be pushed on the top of the stack, representing a new scope.
 */
void scope_enter();

/**
 * Pop the current scope from the symbol table stack.
 *  */
void scope_exit();

/**
 * Insert into the current scope an entry binding a name (identifier) to a symbol object.
 *  */
void scope_bind(const char* name, struct symbol* s);

/**
 * Searches the stack of hash tables from top to bottom, looking for the first entry that matches name exactly. If no match is found, it returns null.
 */
struct symbol* scope_lookup(const char* name);

/**
 * Works like scope lookup except that it only searches the topmost table. This is used to determine whether a symbol has already been defined in the current scope.
 */
struct symbol* scope_lookup_current(const char* name);

/**
 * Returns the depth of the symbol table stack.
 */
int scope_level();

#endif