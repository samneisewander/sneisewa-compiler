
#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

#include "type.h"

typedef enum {
    SYMBOL_LOCAL,
    SYMBOL_PARAM,
    SYMBOL_GLOBAL
} symbol_t;

struct symbol {
    symbol_t kind;
    bool func_defined;
    struct type* type;
    char* name;
    int which;
    int local_count;  // If this is a function symbol, how many locals the function definition has.
};

struct symbol* symbol_create(symbol_t kind, struct type* type, char* name);

// TODO: Symbol delete

/**
 * Returns a string which is a fragment of an instruction, representing the address computation needed for a given symbol.
 */
const char* symbol_codegen(struct symbol* s);

#endif
