%{

#include <stdio.h>

extern int yylex();
extern char *yytext;
extern int yylineno;

void yyerror (char const *s);

%}

%locations
%define parse.error verbose

%token TOKEN_LPAREN 
%token TOKEN_RPAREN
%token TOKEN_LBRACKET
%token TOKEN_RBRACKET
%token TOKEN_LCURLY
%token TOKEN_RCURLY
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
%token TOKEN_IDENTIFIER
%token TOKEN_INTEGER_LITERAL                    
%token TOKEN_HEX_LITERAL
%token TOKEN_BINARY_LITERAL
%token TOKEN_DOUBLE
%token TOKEN_DOUBLE_SCIENTIFIC
%token TOKEN_CHARACTER_LITERAL
%token TOKEN_STRING_LITERAL
%token TOKEN_SEMICOLON
%token TOKEN_COMMA
%token TOKEN_COLON
%token TOKEN_ERROR

%%
program : decl.func.list
        ;

// ----- EXPRESSIONS -----
expr    : expr.1 TOKEN_ASSIGNMENT expr
        | expr.1
        ;

expr.1  : expr.1 TOKEN_LOGICAL_OR expr.2
        | expr.2
        ;

expr.2  : expr.2 TOKEN_LOGICAL_AND expr.3
        | expr.3
        ;

expr.3  : expr.3 TOKEN_LESS_THAN expr.4
        | expr.3 TOKEN_LESS_THAN_OR_EQUAL expr.4
        | expr.3 TOKEN_GREATER_THAN expr.4
        | expr.3 TOKEN_GREATER_THAN_OR_EQUAL expr.4
        | expr.3 TOKEN_EQUAL expr.4
        | expr.3 TOKEN_NOT_EQUAL expr.4
        | expr.4
        ;

expr.4  : expr.4 TOKEN_ADD expr.5
        | expr.4 TOKEN_SUBTRACT expr.5
        | expr.5
        ;

expr.5  : expr.5 TOKEN_MULTIPLY expr.6  
        | expr.5 TOKEN_DIVIDE expr.6
        | expr.5 TOKEN_REMAINDER expr.6
        | expr.6
        ;

expr.6  : expr.6 TOKEN_EXPONENT expr.7
        | expr.7
        ;

expr.7  : TOKEN_SUBTRACT expr.7
        | TOKEN_LOGICAL_NOT expr.7
        | expr.8
        ;

expr.8  : TOKEN_UNARY_ARRAY_LENGTH expr.8
        | expr.9
        ;

expr.9  : expr.10 TOKEN_POSTFIX_INCREMENT
        | expr.10 TOKEN_POSTFIX_DECREMENT
        | expr.10
        ;

expr.10 : TOKEN_LPAREN expr TOKEN_RPAREN
        | TOKEN_IDENTIFIER expr.index
        | TOKEN_IDENTIFIER expr.function
        | TOKEN_INTEGER_LITERAL
        | TOKEN_HEX_LITERAL
        | TOKEN_BINARY_LITERAL
        | TOKEN_DOUBLE
        | TOKEN_DOUBLE_SCIENTIFIC
        | TOKEN_KEY_TRUE
        | TOKEN_KEY_FALSE
        | TOKEN_CHARACTER_LITERAL
        | TOKEN_STRING_LITERAL
        ;

expr.index      : %empty
                | TOKEN_LBRACKET expr TOKEN_RBRACKET expr.index
                ;

expr.function   : TOKEN_LPAREN expr.list.opt TOKEN_RPAREN
                ;

expr.list       : expr expr.list.item
                ;

expr.list.item  : %empty
                | TOKEN_COMMA expr.list
                ;

expr.recursive.list     : expr.list
                        | TOKEN_LCURLY expr.recursive.list TOKEN_RCURLY expr.recursive.list.item
                        ;

expr.recursive.list.item: %empty
                        | TOKEN_COMMA expr.recursive.list

expr.list.opt   : %empty
                | expr.list
                ;

expr.opt        : %empty
                | expr
                ;

// ----- STATEMENTS -----

stmt    : stmt.if.complex
        | stmt.if.simple 
        | stmt.for.simple
        | stmt.for.complex
        ;

// Dangling Else Problem: Need to prevent simple if statement from appearing inside complex if statement.

// Nested complex if statement inside complex if statement, or daisy chaining complex if statements.
stmt.if.complex : TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.if.complex TOKEN_KEY_ELSE stmt.if.complex
                | TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.for.complex TOKEN_KEY_ELSE stmt.if.complex
                | stmt.other
                ;

// Nested complex if or simple if statement inside simple if statement, or daisy chaining simple if statements
stmt.if.simple  : TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt
                | TOKEN_KEY_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt.if.complex TOKEN_KEY_ELSE stmt.if.simple
                ;

stmt.other      : expr TOKEN_SEMICOLON;
                | TOKEN_LCURLY stmt.list.opt TOKEN_RCURLY
                | TOKEN_KEY_PRINT expr.list.opt TOKEN_SEMICOLON
                | TOKEN_KEY_RETURN expr.opt TOKEN_SEMICOLON
                ;

stmt.for.complex        : TOKEN_KEY_FOR TOKEN_LPAREN expr.opt TOKEN_SEMICOLON expr.opt TOKEN_SEMICOLON expr.opt TOKEN_RPAREN stmt.if.complex
                        ;

stmt.for.simple         : TOKEN_KEY_FOR TOKEN_LPAREN expr.opt TOKEN_SEMICOLON expr.opt TOKEN_SEMICOLON expr.opt TOKEN_RPAREN stmt.if.simple
                        ;

stmt.list               : stmt stmt.list.opt

stmt.list.opt           : %empty
                        | stmt.list
                        ;

// ----- DECLARATIONS -----

decl            : decl.array
                | decl.atomic
                ;

decl.func       : decl
                | func
                ;

decl.func.list  : %empty
                | decl.func decl.func.list
                ;

decl.atomic     : TOKEN_IDENTIFIER TOKEN_COLON types.func.params decl.atomic.init TOKEN_SEMICOLON

decl.array      : TOKEN_IDENTIFIER TOKEN_COLON types.array decl.array.init TOKEN_SEMICOLON        
                ;

decl.array.init : %empty
                | TOKEN_ASSIGNMENT expr.recursive.list
                ;

decl.atomic.init: %empty
                | TOKEN_ASSIGNMENT expr
                ;

// ----- FUNCTIONS -----

func            : TOKEN_IDENTIFIER TOKEN_COLON TOKEN_KEY_FUNCTION types.return TOKEN_LPAREN func.params.list TOKEN_RPAREN func.def_or_semi
                ;

func.def        : %empty
                | decl func.def
                | stmt func.def
                ;

func.def_or_semi: TOKEN_SEMICOLON
                | TOKEN_LCURLY func.def TOKEN_RCURLY
                ;

func.params.list: func.params func.params.list.opt
                ;

func.params.list.opt    : %empty
                        | TOKEN_COMMA func.params func.params.list.opt
                        ;
                        
func.params     : %empty
                | TOKEN_IDENTIFIER TOKEN_COLON types.array
                | TOKEN_IDENTIFIER TOKEN_COLON types.func.params
                ;

// ----- TYPES -----

types.atomic    : TOKEN_KEY_BOOLEAN
                | TOKEN_KEY_CHAR
                | TOKEN_KEY_DOUBLE
                | TOKEN_KEY_INTEGER
                | TOKEN_KEY_STRING
                ;

types.func.params       : types.atomic
                        | TOKEN_KEY_AUTO
                        ;

types.array.key : TOKEN_KEY_CARRAY
                | TOKEN_KEY_ARRAY
                | TOKEN_KEY_AUTO
                ;

types.array     : types.array.key TOKEN_LBRACKET expr TOKEN_RBRACKET types.array.member
                ;

types.return    : types.atomic
                | TOKEN_KEY_VOID
                | TOKEN_KEY_AUTO
                ;

types.array.member      : types.atomic
                        | types.array
                        | TOKEN_KEY_AUTO
                        ;

%%

void yyerror (char const *s) {
  fprintf (stderr, "%d: %s\n", yylineno, s);
}