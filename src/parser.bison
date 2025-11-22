%{

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "decl.h"
#include "expr.h"
#include "param_list.h"
#include "stmt.h"
#include "type.h"

extern int yylex();
extern char *yytext;
extern int yylineno;

void yyerror (char const *s);

struct decl *program_ast;

%}

/**
* This union describes nodes in the AST, which can be any of these data types. 
*/
%union {
	struct decl *decl;
	struct stmt *stmt;
	struct expr *expr;
	struct type *type;
	struct param_list *params;

	int64_t int_literal;
	double double_literal;
	char char_literal;
	char *string_literal;
	char *identifier;
	char *name;
};

%locations

%define parse.error verbose

// Grouping
%token TOKEN_LPAREN 
%token TOKEN_RPAREN
%token TOKEN_LBRACKET
%token TOKEN_RBRACKET
%token TOKEN_LCURLY
%token TOKEN_RCURLY

// Operators
%token TOKEN_ADD                       
%token TOKEN_SUBTRACT
%token TOKEN_MULTIPLY
%token TOKEN_DIVIDE
%token TOKEN_EXPONENT
%token TOKEN_REMAINDER
%token TOKEN_POSTFIX_INCREMENT
%token TOKEN_POSTFIX_DECREMENT
%token TOKEN_UNARY_ARRAY_LENGTH
%token TOKEN_LESS_THAN
%token TOKEN_LESS_THAN_OR_EQUAL
%token TOKEN_EQUAL
%token TOKEN_GREATER_THAN
%token TOKEN_GREATER_THAN_OR_EQUAL
%token TOKEN_NOT_EQUAL
%token TOKEN_LOGICAL_NOT                        
%token TOKEN_LOGICAL_AND
%token TOKEN_LOGICAL_OR
%token TOKEN_ASSIGNMENT

// Keywords
%token TOKEN_KEY_ARRAY
%token TOKEN_KEY_AUTO
%token TOKEN_KEY_BOOLEAN
%token TOKEN_KEY_CARRAY
%token TOKEN_KEY_CHAR
%token TOKEN_KEY_ELSE
%token TOKEN_KEY_FALSE
%token TOKEN_KEY_FLOAT
%token TOKEN_KEY_DOUBLE
%token TOKEN_KEY_FOR
%token TOKEN_KEY_FUNCTION
%token TOKEN_KEY_IF
%token TOKEN_KEY_INTEGER
%token TOKEN_KEY_PRINT
%token TOKEN_KEY_RETURN
%token TOKEN_KEY_STRING
%token TOKEN_KEY_TRUE
%token TOKEN_KEY_VOID
%token TOKEN_KEY_WHILE

// Literals
%token <name> TOKEN_IDENTIFIER
%token TOKEN_INTEGER_LITERAL                    
%token TOKEN_HEX_LITERAL
%token TOKEN_BINARY_LITERAL
%token TOKEN_DOUBLE
%token TOKEN_DOUBLE_SCIENTIFIC
%token TOKEN_CHARACTER_LITERAL
%token TOKEN_STRING_LITERAL

// Punctuation
%token TOKEN_SEMICOLON
%token TOKEN_COMMA
%token TOKEN_COLON
%token TOKEN_ERROR

/**
* Associates the semantic values of all rule reductions in the grammar 
* with node types in the abstract syntax tree. 
*/
%type <decl> program decl decl.list 
%type <stmt> stmt stmt.if.complex stmt.if.simple stmt.other stmt.for.complex stmt.for.simple stmt.list stmt.list.opt func.def func.def_or_semi
%type <expr> expr expr.1 expr.2 expr.3 expr.4 expr.5 expr.6 expr.7 expr.8 expr.9 expr.10 expr.function arg.list arg.list.item array.init array.init.item arg.list.opt expr.opt expr.subscript.opt decl.init array.list array.list.item
%type <type> types.atomic types.array.key types.array types.return types.array.member types.decl
%type <params> func.params func.params.list func.params.list.opt
%type <name> expr.ident

%%

program : decl.list { program_ast = $$ = $1;}
        ;

// ----- DECLARATIONS -----

// you should allow any atomic or array type to be assigned any kind of thing...
decl            : expr.ident TOKEN_COLON types.decl decl.init TOKEN_SEMICOLON      
				  { $$ = decl_create($1, $3, $4, 0, 0); }
				| expr.ident TOKEN_COLON TOKEN_KEY_FUNCTION types.return TOKEN_LPAREN func.params.list TOKEN_RPAREN func.def_or_semi
		  		  { $$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, $8, 0); }
                ;

decl.list  		: %empty						
				  { $$ = 0; }
                | decl decl.list		
				  { $$ = $1; $1->next = $2; }
                ;

// From the spec: (of a decl) If no initializing value is given, default is zero
decl.init	: %empty
			  { $$ = 0; }
			| TOKEN_ASSIGNMENT TOKEN_LCURLY array.init TOKEN_RCURLY
			  { $$ = expr_create(EXPR_ARRAY_INIT, $3, 0); }
			| TOKEN_ASSIGNMENT expr
			  { $$ = $2; }
			;


// ----- FUNCTIONS -----

func.def        	: %empty
		  			  { $$ = 0; }
                	| stmt func.def
		  			  { $1->next = $2; $$ = $1; }
                	;

func.def_or_semi	: TOKEN_SEMICOLON
		  			  { $$ = 0; }
                	| TOKEN_ASSIGNMENT TOKEN_LCURLY func.def TOKEN_RCURLY
		  			  { $$ = stmt_create(STMT_BLOCK, 0, 0, 0, 0, $3, 0, 0); }
                	;

func.params.list	: %empty
					  { $$ = 0; }
					| func.params func.params.list.opt
		  			  { $1->next = $2; $$ = $1; }
                	;

func.params.list.opt: %empty
		  			  { $$ = 0; }
        			| TOKEN_COMMA func.params func.params.list.opt
		  			  { $2->next = $3; $$ = $2; }
        			;
                        
func.params     	: expr.ident TOKEN_COLON types.array
		  			  { $$ = param_list_create($1, $3, 0); }
					| expr.ident TOKEN_COLON types.atomic
		  			  { $$ = param_list_create($1, $3, 0); }
					;

// ----- STATEMENTS -----
stmt    		: stmt.if.complex
				  { $$ = $1; }
        		| stmt.if.simple 
				  { $$ = $1; }
        		| stmt.for.simple
				  { $$ = $1; }
        		| stmt.for.complex
				  { $$ = $1; }
				| decl
				  { $$ = stmt_create(STMT_DECL, $1, 0, 0, 0, 0, 0, 0); }
        		;

// Dangling Else Problem: Need to prevent simple if statement from appearing inside complex if statement.

// Case: A nested complex if inside a complex if, or daisy chaining complex if statements.
stmt.if.complex : TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.if.complex TOKEN_KEY_ELSE stmt.if.complex
				  { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0); }
                | TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.for.complex TOKEN_KEY_ELSE stmt.if.complex
				  { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0); }
                | stmt.other
				  { $$ = $1; }
                ;

// Case: A nested complex if or simple if statement inside a simple if statement, or daisy chaining simple if statements
stmt.if.simple  : TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt
				  { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, 0, 0); }
                | TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.if.complex TOKEN_KEY_ELSE stmt.if.simple
				  { $$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0); }
                ;

stmt.other      : expr TOKEN_SEMICOLON
				  { $$ = stmt_create(STMT_EXPR, 0, 0, $1, 0, 0, 0, 0); }
                | TOKEN_LCURLY stmt.list.opt TOKEN_RCURLY
				  { $$ = stmt_create(STMT_BLOCK, 0, 0, 0, 0, $2, 0, 0); }
                | TOKEN_KEY_PRINT arg.list.opt TOKEN_SEMICOLON
				  { $$ = stmt_create(STMT_PRINT, 0, 0, $2, 0, 0, 0, 0); }
                | TOKEN_KEY_RETURN expr.opt TOKEN_SEMICOLON
				  { $$ = stmt_create(STMT_RETURN, 0, 0, $2, 0, 0, 0, 0); }
                ;

// TODO: Elaborate on complication with for loops and if statements
stmt.for.complex        : TOKEN_KEY_FOR TOKEN_LPAREN expr.opt TOKEN_SEMICOLON expr.opt TOKEN_SEMICOLON expr.opt TOKEN_RPAREN stmt.if.complex
				  		  { $$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0); }
                        ;

// TODO: Elaborate on complication with for loops and if statements
stmt.for.simple         : TOKEN_KEY_FOR TOKEN_LPAREN expr.opt TOKEN_SEMICOLON expr.opt TOKEN_SEMICOLON expr.opt TOKEN_RPAREN stmt.if.simple
				  		  { $$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0); }
                        ;

stmt.list               : stmt stmt.list.opt
				  		  { $1->next = $2; $$ = $1; }
						;

stmt.list.opt           : %empty
				  		  { $$ = 0; }
                        | stmt.list
				  		  { $$ = $1; }
                        ;

// ----- EXPRESSIONS -----
expr    : expr.1 TOKEN_ASSIGNMENT expr	
		  { $$ = expr_create(EXPR_ASSIGNMENT, $1, $3); }
        | expr.1
		  { $$ = $1; }
        ;

expr.1  : expr.1 TOKEN_LOGICAL_OR expr.2
		  { $$ = expr_create(EXPR_LOGICAL_OR, $1, $3); }
        | expr.2
		  { $$ = $1; }
        ;

expr.2  : expr.2 TOKEN_LOGICAL_AND expr.3
		  { $$ = expr_create(EXPR_LOGICAL_AND, $1, $3); }
        | expr.3
		  { $$ = $1; }
        ;

expr.3  : expr.3 TOKEN_LESS_THAN expr.4
		  { $$ = expr_create(EXPR_LESS_THAN, $1, $3); }
        | expr.3 TOKEN_LESS_THAN_OR_EQUAL expr.4
		  { $$ = expr_create(EXPR_LESS_THAN_OR_EQUAL, $1, $3); }
        | expr.3 TOKEN_EQUAL expr.4
		  { $$ = expr_create(EXPR_EQUAL, $1, $3); }
        | expr.3 TOKEN_GREATER_THAN expr.4
		  { $$ = expr_create(EXPR_GREATER_THAN, $1, $3); }
        | expr.3 TOKEN_GREATER_THAN_OR_EQUAL expr.4
		  { $$ = expr_create(EXPR_GREATER_THAN_OR_EQUAL, $1, $3); }
        | expr.3 TOKEN_NOT_EQUAL expr.4
		  { $$ = expr_create(EXPR_NOT_EQUAL, $1, $3); }
        | expr.4
		  { $$ = $1; }
        ;

expr.4  : expr.4 TOKEN_ADD expr.5
		  { $$ = expr_create(EXPR_ADD, $1, $3); }
        | expr.4 TOKEN_SUBTRACT expr.5
		  { $$ = expr_create(EXPR_SUBTRACT, $1, $3); }
        | expr.5
		  { $$ = $1; }
        ;

expr.5  : expr.5 TOKEN_MULTIPLY expr.6  
		  { $$ = expr_create(EXPR_MULTIPLY, $1, $3); }
        | expr.5 TOKEN_DIVIDE expr.6
		  { $$ = expr_create(EXPR_DIVIDE, $1, $3); }
        | expr.5 TOKEN_REMAINDER expr.6
		  { $$ = expr_create(EXPR_REMAINDER, $1, $3); }
        | expr.6
		  { $$ = $1; }
        ;

expr.6  : expr.7 TOKEN_EXPONENT expr.6
		  { $$ = expr_create(EXPR_EXPONENT, $1, $3); }
        | expr.7
		  { $$ = $1; }
        ;

expr.7  : TOKEN_SUBTRACT expr.7
		  { $$ = expr_create(EXPR_UNARY_NEGATION, $2, 0); }
        | TOKEN_LOGICAL_NOT expr.7
		  { $$ = expr_create(EXPR_LOGICAL_NOT, $2, 0); }
        | expr.8
		  { $$ = $1; }
        ;

expr.8  : TOKEN_UNARY_ARRAY_LENGTH expr.8
		  { $$ = expr_create(EXPR_UNARY_ARRAY_LENGTH, $2, 0); }
        | expr.9
		  { $$ = $1; }
        ;

expr.9  : expr.9 TOKEN_POSTFIX_INCREMENT
		  { $$ = expr_create(EXPR_POSTFIX_INCREMENT, $1, 0); }
        | expr.9 TOKEN_POSTFIX_DECREMENT
		  { $$ = expr_create(EXPR_POSTFIX_DECREMENT, $1, 0); }
        | expr.10
		  { $$ = $1; }
        ;

expr.10 : TOKEN_LPAREN expr TOKEN_RPAREN
		  { $$ = $2; }
		| expr.ident 
		  { $$ = expr_create_name($1); }
        | expr.subscript.opt
		  { $$ = $1; }
        | expr.ident expr.function
		  { $$ = expr_create(EXPR_CALL, expr_create_name($1), $2); }
        | TOKEN_INTEGER_LITERAL
		  { $$ = expr_create_integer_literal(yylval.int_literal); }
        | TOKEN_HEX_LITERAL
		  { $$ = expr_create_integer_literal(yylval.int_literal); }
        | TOKEN_BINARY_LITERAL
		  { $$ = expr_create_integer_literal(yylval.int_literal); }
        | TOKEN_DOUBLE
		  { $$ = expr_create_double_literal(yylval.double_literal); }
        | TOKEN_DOUBLE_SCIENTIFIC
		  { $$ = expr_create_double_literal(yylval.double_literal); }
        | TOKEN_KEY_TRUE
		  { $$ = expr_create_boolean_literal(1); }
        | TOKEN_KEY_FALSE
		  { $$ = expr_create_boolean_literal(0); }
        | TOKEN_CHARACTER_LITERAL
		  { $$ = expr_create_char_literal(yylval.char_literal); }
        | TOKEN_STRING_LITERAL
		  { $$ = expr_create_string_literal(yylval.string_literal); }
        ;

expr.ident 	: TOKEN_IDENTIFIER
			  { $$ = strdup(yylval.identifier); free(yylval.identifier); }
			;

expr.subscript.opt		: expr.ident TOKEN_LBRACKET expr TOKEN_RBRACKET
						  { $$ = expr_create(EXPR_SUBSCRIPT, expr_create_name($1), $3); }
						| expr.subscript.opt TOKEN_LBRACKET expr TOKEN_RBRACKET
						  { $$ = expr_create(EXPR_SUBSCRIPT, $1, $3); }
						;

expr.function   		: TOKEN_LPAREN arg.list.opt TOKEN_RPAREN
		  				  { $$ = $2; }
                		;

// Linked list for function arguments and print statements.
arg.list.opt   			: %empty
		  				  { $$ = 0; }
                		| arg.list
		  				  { $$ = $1; }

arg.list       			: expr arg.list.item
		  				  { $$ = expr_create(EXPR_ARG, $1, $2); }
                		;

arg.list.item  			: %empty
		  				  { $$ = 0; }
                		| TOKEN_COMMA arg.list
		  				  { $$ = $2; }
                		;

// Linked list of array elements (has the whack ass recursive stuff, and different expr kind)
array.init     			: expr array.list.item
		  				  { $$ = expr_create(EXPR_ARRAY_ELEM, $1, $2); }
                        | TOKEN_LCURLY array.init TOKEN_RCURLY array.init.item
		  				  { $$ = expr_create(EXPR_ARRAY_INIT, $2, $4); }
                        ;

array.list       		: expr array.list.item
		  				  { $$ = expr_create(EXPR_ARRAY_ELEM, $1, $2); }
                		;

array.list.item  		: %empty
		  				  { $$ = 0; }
                		| TOKEN_COMMA array.list
		  				  { $$ = $2; }
                		;

array.init.item			: %empty
		  				  { $$ = 0; }
                        | TOKEN_COMMA array.init
		  				  { $$ = $2; }
						;

expr.opt        		: %empty
		  				  { $$ = 0; }
                		| expr
		  				  { $$ = $1; }
                		;

// ----- TYPES -----
types.atomic    		: TOKEN_KEY_BOOLEAN
						  { $$ = type_create(TYPE_BOOLEAN, 0, 0, 0); }
                		| TOKEN_KEY_CHAR
						  { $$ = type_create(TYPE_CHARACTER, 0, 0, 0); }
                		| TOKEN_KEY_DOUBLE
						  { $$ = type_create(TYPE_DOUBLE, 0, 0, 0); }
                		| TOKEN_KEY_INTEGER
						  { $$ = type_create(TYPE_INTEGER, 0, 0, 0); }
                		| TOKEN_KEY_STRING
						  { $$ = type_create(TYPE_STRING, 0, 0, 0); }
						| TOKEN_KEY_AUTO
						  { $$ = type_create(TYPE_AUTO, 0, 0, 0); }
                		;

types.array.key 		: TOKEN_KEY_CARRAY
						  { $$ = type_create(TYPE_CARRAY, 0, 0, 0); }
                		| TOKEN_KEY_ARRAY
						  { $$ = type_create(TYPE_ARRAY, 0, 0, 0); }
                		;

types.array     		: types.array.key TOKEN_LBRACKET expr TOKEN_RBRACKET types.array.member
						  { $1->subtype = $5; $1->array_size = $3; $$ = $1; }
                		;

types.decl				: types.array
						  { $$ = $1; }
						| types.atomic
						  { $$ = $1; }
						;

types.return    		: types.atomic
						  { $$ = $1; }
                		| TOKEN_KEY_VOID
						  { $$ = type_create(TYPE_VOID, 0, 0, 0); }
                		;

types.array.member      : types.atomic
				  		  { $$ = $1; }
                        | types.array
				  		  { $$ = $1; }
                        ;

%%

void yyerror (char const *s) {
  fprintf (stderr, "%d: %s\n", yylineno, s);
}