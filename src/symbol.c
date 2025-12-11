/* symbol.c */

#include "symbol.h"
#define MAX_SYMBOL_NAME 256

#include <stdlib.h>

struct symbol* symbol_create(symbol_t kind, struct type* type, char* name) {
    struct symbol* s = calloc(1, sizeof(*s));

    s->kind = kind;
    s->type = type;
    s->name = name;

    return s;
}

const char* symbol_codegen(struct symbol* s) {
    static char name[MAX_SYMBOL_NAME];
    const char* arg_reg_map[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    switch (s->kind) {
        case SYMBOL_GLOBAL:
            /* Names for globals in assembly language are the same as the names in the source language. For example, if we have `count: integer`, then simply return "count". */
            snprintf(name, MAX_SYMBOL_NAME, s->name);
            return name;
        case SYMBOL_LOCAL:
            /**
             * Each local occupies one q-word (8 bytes), and they appear immediately after the base pointer, growing towards smaller addresses. Therefore, to compute the position of this local, we need to mulitply the offset (1 + the which value) to the size of a negative q-word (since the stack grows downwards).
             */
            snprintf(name, MAX_SYMBOL_NAME, "-%d(%%rbp)", 8 * (1 + s->which));
            return name;
        case SYMBOL_PARAM:
            snprintf(name, MAX_SYMBOL_NAME, "%s", arg_reg_map[s->which]);
            return name;
        default:
            return "";
    }
}