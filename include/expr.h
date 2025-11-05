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

    /* used by various leaf exprs */
    const char* name;
    int64_t int_literal;
    bool bool_literal;
    double double_literal;
    char char_literal;
    const char* string_literal;
    struct symbol* symbol;
};

struct expr* expr_create(expr_t kind, struct expr* left, struct expr* right);

struct expr* expr_create_name(const char* n);
struct expr* expr_create_integer_literal(int64_t c);
struct expr* expr_create_double_literal(double c);
struct expr* expr_create_boolean_literal(bool c);
struct expr* expr_create_char_literal(char c);
struct expr* expr_create_string_literal(const char* str);

void expr_print(struct expr* e);
void expr_print_parens(struct expr* e);
#endif
