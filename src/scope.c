/* scope.c */

#include "scope.h"

#include <stdbool.h>

#include "error.h"
#include "hash_table.h"
#include "symbol.h"

typedef struct Node {
    struct hash_table* symbol_table;
    struct Node* next;
    scope_t scope_kind;
    int symbol_count;
} Node;

typedef struct {
    Node* head;
    size_t size;
} Stack;

Stack Symbol_table_stack = {.size = 0, .head = NULL};

void scope_enter(scope_t scope_kind) {
    Node* n = (Node*)calloc(1, sizeof(*n));
    n->symbol_table = hash_table_create(0, 0);

    // Do we carry over "which", or reset? If this scope is for a function, need reset.
    // if this scope is for a STMT_BLOCK, then it should carry over.
    n->scope_kind = scope_kind;
    if (scope_kind == SCOPE_STMT_BLOCK) {
        n->symbol_count = Symbol_table_stack.head->symbol_count;
    }

    if (Symbol_table_stack.head) {
        n->next = Symbol_table_stack.head;
        Symbol_table_stack.head = n;
    } else {
        Symbol_table_stack.head = n;
    }

    Symbol_table_stack.size++;
}

void scope_exit() {
    Node* head = Symbol_table_stack.head;
    if (!head) fprintf(stderr, "resolver: cannot exit scope without first entering one.\n");

    // Carry over symbol "which" count if we are exiting a STMT_BLOCK
    if (head->scope_kind == SCOPE_STMT_BLOCK) {
        head->next->symbol_count = head->symbol_count;
    }

    if (head->next) {
        Symbol_table_stack.head = head->next;
        head->next = NULL;
    }

    hash_table_delete(head->symbol_table);
    free(head);

    Symbol_table_stack.size--;
}

void scope_bind(const char* name, struct symbol* s) {
    if (!Symbol_table_stack.head) fprintf(stderr, "resolver: attempt to bind name without first entering a scope.\n");

    struct hash_table* scope = Symbol_table_stack.head->symbol_table;
    s->which = Symbol_table_stack.head->symbol_count++;

    int status = hash_table_insert(scope, name, s);
    if (!status) {
        fprintf(stderr, "resolver: could not bind symbol to name %s\n", name);
        set_error(true);
    }
}

struct symbol* scope_lookup(const char* name) {
    struct symbol* sym = NULL;
    char* symbol_kind = NULL;

    Node* head = Symbol_table_stack.head;
    while (head) {
        sym = hash_table_lookup(head->symbol_table, name);
        if (sym) {
            switch (sym->kind) {
                case SYMBOL_LOCAL:
                    symbol_kind = "local";
                    break;
                case SYMBOL_GLOBAL:
                    symbol_kind = "global";
                    break;
                case SYMBOL_PARAM:
                    symbol_kind = "param";
                    break;
            }
            printf("%s resolves to %s %d\n", name, symbol_kind, sym->which);
            return sym;
        }
        head = head->next;
    }

    return NULL;
}

struct symbol* scope_lookup_current(const char* name) {
    struct symbol* sym = NULL;
    char* symbol_kind = NULL;

    Node* head = Symbol_table_stack.head;
    sym = hash_table_lookup(head->symbol_table, name);
    if (sym) {
        switch (sym->kind) {
            case SYMBOL_LOCAL:
                symbol_kind = "local";
                break;
            case SYMBOL_GLOBAL:
                symbol_kind = "global";
                break;
            case SYMBOL_PARAM:
                symbol_kind = "param";
                break;
        }
        return sym;
    }

    return NULL;
}

int scope_level() {
    return Symbol_table_stack.size;
}