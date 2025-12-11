#ifndef EXPR_H
#define EXPR_H

#include <stdbool.h>
#include <stdint.h>

#include "symbol.h"

/**
 * Kinds of expressions. Operators are ordered by precedence,
 * where higher number means higher precedence. Kinds that not
 * seperated by whitespace have equal precedence. Printing
 * functions rely on this ordering; it must be preserved.
 *
 * Similarly, atomic expressions and names of identifiers must
 * appear last.
 */
typedef enum {
    EXPR_ASSIGNMENT,

    EXPR_LOGICAL_OR,

    EXPR_LOGICAL_AND,

    EXPR_LESS_THAN,
    EXPR_LESS_THAN_OR_EQUAL,
    EXPR_EQUAL,
    EXPR_GREATER_THAN,
    EXPR_GREATER_THAN_OR_EQUAL,
    EXPR_NOT_EQUAL,

    EXPR_ADD,
    EXPR_SUBTRACT,

    EXPR_MULTIPLY,
    EXPR_DIVIDE,
    EXPR_REMAINDER,

    EXPR_EXPONENT,

    EXPR_UNARY_NEGATION,
    EXPR_LOGICAL_NOT,

    EXPR_UNARY_ARRAY_LENGTH,

    EXPR_POSTFIX_INCREMENT,
    EXPR_POSTFIX_DECREMENT,

    EXPR_SUBSCRIPT,
    EXPR_CALL,
    EXPR_ARG,
    EXPR_ARRAY_ELEM,
    EXPR_ARRAY_INIT,

    EXPR_NAME,
    EXPR_INT,
    EXPR_DOUBLE,
    EXPR_BOOLEAN,
    EXPR_CHAR,
    EXPR_STRING,
} expr_t;

struct expr {
    /* used by all kinds of exprs */
    expr_t kind;
    struct expr* left;
    struct expr* right;
    int reg;  // The register holding the computed value of this expression.

    /* used by various leaf exprs */
    char* name;
    int64_t int_literal;
    bool bool_literal;
    double double_literal;
    char char_literal;
    char* string_literal;
    struct symbol* symbol;
};

struct expr* expr_create(expr_t kind, struct expr* left, struct expr* right);

struct expr* expr_create_name(char* n);
struct expr* expr_create_integer_literal(int64_t c);
struct expr* expr_create_double_literal(double c);
struct expr* expr_create_boolean_literal(bool c);
struct expr* expr_create_char_literal(char c);
struct expr* expr_create_string_literal(char* str);

/**
 * Pretty prints an expr, removing unnecessary whitespace or parentheses while preserving the structure of the AST. If expr is null, does nothing.
 */
void expr_print(struct expr* e, FILE* fp);

/**
 * Pretty prints expr e, wrapped in parentheses.
 */
void expr_print_parens(struct expr* e, FILE* fp);

/**
 * @brief Performs name resolution on an expression.
 *
 * Creates a new symbol, binding it to the name of the declaration in the current scope. If the declaration represents an expression (d->value is not null) then the expression is resolved. If the declaration represents a function (d->code is not null), creates a new scope and resolves the parameters and code.
 * @param d decl on which to perform name resolution.
 */
void expr_resolve(struct expr* e);

/**
 * Recursively copies an expression `e` and returns the result. If `e` is `NULL`, returns `NULL`. Note that the symbol structure associated with `e` is reused.
 */
struct expr* expr_copy(struct expr* e);

/**
 * Recursively releases expression `e` and its members. If `e` is `NULL`, does nothing. Note that this does not release the symbol structure associated with `e`.
 */
void expr_delete(struct expr* e);

/**
 * If called on a non-null expr, will always return a newly-allocated type structure.
 */
struct type* expr_typecheck(struct expr* e);

/**
 * Perform a post-order traversal of the AST or DAG starting at e, and emit one or more instructions for each node.
 */
void expr_codegen(struct expr* e, FILE* outfile);

#endif
