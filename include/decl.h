
#ifndef DECL_H
#define DECL_H

#include "type.h"
#include "stmt.h"
#include "expr.h"
#include <stdio.h>

/**
 * The Abstract Syntax Tree node kind for a declaration
 */
struct decl {
	char *name;				
	struct type *type;		
	struct expr *value;
	struct stmt *code;
	struct symbol *symbol;
	struct decl *next;
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
struct decl * decl_create( char *name, struct type *type, struct expr *value, struct stmt *code, struct decl *next );
void decl_print( struct decl *d, int indent );

#endif


