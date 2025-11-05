%{
	#include <stdbool.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <errno.h>
    #include <stdint.h>

	#include "token.h"
	#include "encoder.h"

	#define MAX_STR_LEN 255

	bool string_decode(const char *es, char *s);
%}

DIGIT 				[0-9]

HEX					[0-9a-fA-F]
LETTER 				[a-zA-Z]
ALPHANUM 			[a-zA-Z0-9]

ESCAPE_SEQUENCE   	\\([abefrntv\\\'\"]|0x{HEX}{HEX})
SYMBOL				[ !\#$\%&'\(\)\*\+,\-\.\/:;<=>?@\[\]\^_`\{|\}~]

CHARACTER			({ALPHANUM}|{ESCAPE_SEQUENCE}|{SYMBOL})

INTEGER_LITERAL 	{DIGIT}*
HEX_LITERAL 		0x{HEX}*
BINARY_LITERAL 		0b[01]*

DOUBLE				{INTEGER_LITERAL}\.{DIGIT}+
DOUBLE_SCIENTIFIC	{INTEGER_LITERAL}\.?{DIGIT}*(e|E)[+-]?{DIGIT}+

COMMENT_C_STYLE		\/\*(\*[^\/]|[^\*]\/|[^\*\/])*(\*\/)?
COMMENT_CPP_STYLE	\/\/[^\n]*

COMMENT				{COMMENT_C_STYLE}|{COMMENT_CPP_STYLE}

%option yylineno

%%

(" "|\t|\n)         				/* skip whitespace */
{COMMENT}							/* skip comments */

\(  								{ return TOKEN_LPAREN; }   
\)    								{ return TOKEN_RPAREN; }   
\[    								{ return TOKEN_LBRACKET; }   
\]   								{ return TOKEN_RBRACKET; }   
\{    								{ return TOKEN_LCURLY; }   
\}    								{ return TOKEN_RCURLY; }   

\+    								{ return TOKEN_ADD; }      
\-   								{ return TOKEN_SUBTRACT; }      
\*    								{ return TOKEN_MULTIPLY; }     
\/    								{ return TOKEN_DIVIDE; }      
\^    								{ return TOKEN_EXPONENT; }      
\%    								{ return TOKEN_REMAINDER; }      

"++"								{ return TOKEN_POSTFIX_INCREMENT; }       
"--"   								{ return TOKEN_POSTFIX_DECREMENT; }       

"#"    								{ return TOKEN_UNARY_ARRAY_LENGTH; }   

"<"    								{ return TOKEN_LESS_THAN; }       
"<="   								{ return TOKEN_LESS_THAN_OR_EQUAL; }      
"=="    							{ return TOKEN_EQUAL; }       
">"    								{ return TOKEN_GREATER_THAN; }       
">="    							{ return TOKEN_GREATER_THAN_OR_EQUAL; }       
"!="    							{ return TOKEN_NOT_EQUAL; }       

"!"    								{ return TOKEN_LOGICAL_NOT; }      
"&&"   								{ return TOKEN_LOGICAL_AND; }      
"||"   								{ return TOKEN_LOGICAL_OR; }       

"="    								{ return TOKEN_ASSIGNMENT; }

array    							{ return TOKEN_KEY_ARRAY; }
auto    							{ return TOKEN_KEY_AUTO; }
boolean    							{ return TOKEN_KEY_BOOLEAN; }
carray                              { return TOKEN_KEY_CARRAY; }
char    							{ return TOKEN_KEY_CHAR; }
else    							{ return TOKEN_KEY_ELSE; }
false    							{ return TOKEN_KEY_FALSE; }
float    							{ return TOKEN_KEY_FLOAT; }
double    							{ return TOKEN_KEY_DOUBLE; }
for    								{ return TOKEN_KEY_FOR; }
function  							{ return TOKEN_KEY_FUNCTION; }
if    								{ return TOKEN_KEY_IF; }
integer    							{ return TOKEN_KEY_INTEGER; }
print    							{ return TOKEN_KEY_PRINT; }
return    							{ return TOKEN_KEY_RETURN; }
string    							{ return TOKEN_KEY_STRING; }
true    							{ return TOKEN_KEY_TRUE; }
void    							{ return TOKEN_KEY_VOID; }
while    							{ return TOKEN_KEY_WHILE; }

({LETTER}|_)({ALPHANUM}|_){0,254} 	{ 

    yylval.identifier = strdup(yytext);

    return TOKEN_IDENTIFIER; 
}

{INTEGER_LITERAL}	  				{ 

    char *end_ptr = NULL;
    int64_t literal = strtoll(yytext, &end_ptr, 10);

    if (*end_ptr != '\0') {
        fprintf(stderr, "scanner: Invalid integer literal.\n");
        return TOKEN_ERROR;
    }

    yylval.int_literal = literal;

    return TOKEN_INTEGER_LITERAL; 
}
{HEX_LITERAL}  						{ 
    
    char *end_ptr = NULL;
    int64_t literal = strtoll(yytext, &end_ptr, 16);

    if (*end_ptr != '\0') {
        fprintf(stderr, "scanner: Invalid hex literal.\n");
        return TOKEN_ERROR;
    }

    yylval.int_literal = literal;

    return TOKEN_HEX_LITERAL; 
}
{BINARY_LITERAL}	  				{ 

    char *end_ptr = NULL;
    int64_t literal = strtoll(yytext + 2, &end_ptr, 2);

    if (*end_ptr != '\0') {
        fprintf(stderr, "scanner: Invalid binary literal.\n");
        return TOKEN_ERROR;
    }

    yylval.int_literal = literal;

    return TOKEN_BINARY_LITERAL; 
}
{DOUBLE}				    		{ 
    
    char *end_ptr = NULL;
    
    errno = 0;
    double literal = strtod(yytext, &end_ptr);

    if (*end_ptr != '\0') {
        fprintf(stderr, "scanner: Invalid double literal.\n");
        return TOKEN_ERROR;
    }

    if (errno == ERANGE) {
        fprintf(stderr, "scanner: Double exceeds floating point limit.\n");
        return TOKEN_ERROR;
    }

    yylval.double_literal = literal;

    return TOKEN_DOUBLE; 
}
{DOUBLE_SCIENTIFIC}			    	{ 
    
    char *end_ptr = NULL;

    errno = 0;
    double literal = strtod(yytext, &end_ptr);

    if (*end_ptr != '\0') {
        fprintf(stderr, "scanner: Invalid scientific double literal.\n");
        return TOKEN_ERROR;
    }

    if (errno == ERANGE) {
        fprintf(stderr, "scanner: Scientific double exceeds floating point limit.\n");
        return TOKEN_ERROR;
    }


    yylval.double_literal = literal;

    return TOKEN_DOUBLE_SCIENTIFIC; 

}
'{CHARACTER}'    					{ 
	
	char buf[MAX_STR_LEN + 1];
	size_t char_literal_len = strlen(yytext);
	
	*yytext = '"';
	*(yytext + char_literal_len - 1) = '"';
	*(yytext + char_literal_len) = '\0';
	
	if (!string_decode(yytext, buf)) {
		fprintf(stderr, "encoder: Invalid character literal.\n");
		return TOKEN_ERROR;
	}

	int i;
	for(i = 0; buf[i]; i++) {
		yytext[i] = buf[i];
	}

	yytext[i] = '\0';
    yylval.char_literal = buf[0];

	return TOKEN_CHARACTER_LITERAL; 

}
\"{CHARACTER}*\"    				{ 

	char buf[MAX_STR_LEN + 1];
	if (!string_decode(yytext, buf)) {
		fprintf(stderr, "encoder: Invalid string literal.\n");
		return TOKEN_ERROR;
	}

	int i;
	for(i = 0; buf[i]; i++) {
		yytext[i] = buf[i];
	}

	yytext[i] = '\0';
    yylval.string_literal = strdup(yytext);

	return TOKEN_STRING_LITERAL;

}

;									{ return TOKEN_SEMICOLON; }
,									{ return TOKEN_COMMA; }
:									{ return TOKEN_COLON; }

.									{ return TOKEN_ERROR; }

%%

int yywrap() { return 1; }