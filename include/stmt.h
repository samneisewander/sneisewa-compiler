
#ifndef STMT_H
#define STMT_H

#include "decl.h"

typedef enum {
    STMT_DECL,
    STMT_EXPR,
    STMT_IF_ELSE,
    STMT_FOR,
    STMT_PRINT,
    STMT_RETURN,
    STMT_BLOCK
} stmt_t;

/** The kind field indicates what kind of statement it is:
 *  - STMT DECL indicates a (local) declaration, and the decl field will point to it.
 * - STMT EXPR indicates an expression statement and the expr field will point to it.
 * - STMT IF ELSE indicates an if-else expression such that the expr field will point to the control expression, the body field to the statements executed if it is true, and the else body field to the state ments executed if it is false.
 * - STMT FOR indicates a for-loop, such that init expr, expr, and next expr are the three expressions in the loop header, and body points to the statements in the loop.
 * - STMT PRINT indicates a print statement, and expr points to the expressions to print.
 * - STMT RETURN indicates a return statement, and expr points to the expression to return.
 * - STMT BLOCK indicates a block of statements inside curly braces, and body points to the contained statements.
 */
struct stmt {
    stmt_t kind;
    struct decl* decl;
    struct expr* init_expr;
    struct expr* expr;
    struct expr* next_expr;
    struct stmt* body;
    struct stmt* else_body;
    struct stmt* next;
    struct symbol* parent_func;
};

struct stmt* stmt_create(stmt_t kind, struct decl* decl, struct expr* init_expr, struct expr* expr, struct expr* next_expr, struct stmt* body, struct stmt* else_body, struct stmt* next);

void stmt_print(struct stmt* s, int indent);

/**
 * Calls the appropriate resolve on each of its sub-components. In the case of a STMT BLOCK, it also enters and leaves a new scope.
 * @param prototype The symbol structure of the function this statement appears inside of.
 */
void stmt_resolve(struct stmt* s, struct symbol* prototype);

/**
 * Evaluates each of a statement's components, and then verifies that types match where needed.
 *
 * @param prototype The symbol structure of the function this statement appears inside of.
 */
void stmt_typecheck(struct stmt* s, struct symbol* prototype);

/**
 * Creates code for control flow statements.
 */
void stmt_codegen(struct stmt* s, struct symbol* prototype, FILE* outfile);

#endif
