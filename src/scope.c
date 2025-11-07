/* scope.c */

#include "scope.h"

#include "hash_table.h"
#include "symbol.h"

typedef struct Node {
    struct hash_table* symbol_table;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct {
    size_t size;
    Node* head;
} Stack;

Stack Symbol_table_stack = {.size = 0, .head = NULL};

/* PRIVATE METHODS */

/**
 * Factory method for the Node structure. User should call node_delete on the
 * pointer returned by this method. Always returns a valid pointer; on
 * error, the program exits.
 */
static Node* node_create() {
    Node* n = (Node*)calloc(1, sizeof(*n));

    if (!n) {
        fprintf(stderr, "Node_create: calloc failure\n");
        exit(EXIT_FAILURE);
    }

    n->symbol_table = hash_table_create(0, 0);

    return n;
}

/**
 * Releases the memory allocated for a Node n. If n is NULL, does nothing.
 * Note that while the node structure is freed and the hash table is
 * deleted, the values in the hash table are not released.
 */
static void node_delete(Node* n) {
    if (!n) return;
    hash_table_delete(n->symbol_table);
    free(n);
}

/**
 * NOTE: Unused, since the symbol table stack is a global in this implementation
 *
 * Factory method for the Stack structure. User should call stack_delete on the
 * pointer returned by this method. Always returns a valid pointer; on
 * error, the program exits.
 */
static Stack* stack_create() {
    Stack* s = (Stack*)calloc(1, sizeof(*s));

    if (!s) {
        fprintf(stderr, "stack_create: calloc failure\n");
        exit(EXIT_FAILURE);
    }

    return s;
}

/**
 * NOTE: Unused, since the symbol table stack is a global in this implementation
 *
 * Releases the memory allocated for a Stack s, including memory allocated
 * for nodes remaining on the stack if there are any. If s is NULL, does
 * nothing.
 */
static void stack_delete(Stack* s) {
    if (!s) return;
    if (s->head) {
        Node* curr = s->head;
        Node* next = NULL;

        while (curr) {
            next = curr->next;
            node_delete(curr);
            curr = next;
        }
    }

    free(s);
}

/**
 * Pushes a Node n onto a Stack s.
 */
static void stack_push(Node* node, Stack* s) {
    if (s->head) {
        node->prev = s->head;
        s->head->next = node;
        s->head = node;
    } else {
        s->head = node;
    }
    s->size++;
}

/**
 * Pops a Node from a Stack s. If the stack is empty, returns NULL. The user
 * is expected to release memory allocated for the popped node by calling
 * node_delete.
 */
static Node* stack_pop(Stack* s) {
    Node* node = NULL;
    if (s->head) {
        node = s->head;
        s->head = s->head->prev;
        node->next = NULL;
        node->prev = NULL;
        s->size--;
    }
    return node;
}

/**
 * Returns the Node at the top of the stack s, or NULL if s is empty.
 */
static Node* stack_top(Stack* s) {
    /**
     * This module returns a Node*, and not a struct hash_table*. This
     * is to facilitate backwards traversal through the stack during scope lookup.
     */
    if (!s) return NULL;
    if (!s->head) return NULL;
    return s->head;
}

/* PUBLIC METHODS */

void scope_enter() {
    Node* n = node_create();
    stack_push(n, &Symbol_table_stack);
}

void scope_exit() {
    Node* n = stack_pop(&Symbol_table_stack);
    node_delete(n);
}

void scope_bind(const char* name, struct symbol* s) {
    Node* n = stack_top(&Symbol_table_stack);
    struct hash_table* scope = n->symbol_table;
    int status = hash_table_insert(scope, name, s);
    if (!status) {
        fprintf(stderr, "scope_bind: Could not bind symbol to name %s\n", name);
    }
}

struct symbol* scope_lookup(const char* name) {
    Node* node = stack_top(&Symbol_table_stack);
    struct symbol* sym = NULL;

    while (node) {
        sym = hash_table_lookup(node->symbol_table, name);
        if (sym) {
            return sym;
        }
        node = node->prev;
    }

    return NULL;
}

struct symbol* scope_lookup_current(const char* name) {
    Node* node = stack_top(&Symbol_table_stack);
    struct symbol* sym = hash_table_lookup(node->symbol_table, name);

    if (sym) {
        return sym;
    }

    return NULL;
}

int scope_level() {
    return Symbol_table_stack.size;
}