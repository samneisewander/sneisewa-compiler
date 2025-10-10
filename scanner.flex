%{
	#include <stdbool.h>
    #include <stdlib.h>
    #include <stdio.h>

	#include "token.h"

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

({LETTER}|_)({ALPHANUM}|_){0,254} 	{ return TOKEN_IDENTIFIER; }

{INTEGER_LITERAL}	  				{ return TOKEN_INTEGER_LITERAL; }
{HEX_LITERAL}  						{ return TOKEN_HEX_LITERAL; }
{BINARY_LITERAL}	  				{ return TOKEN_BINARY_LITERAL; }
{DOUBLE}				    		{ return TOKEN_DOUBLE; }
{DOUBLE_SCIENTIFIC}			    	{ return TOKEN_DOUBLE_SCIENTIFIC; }
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

	return TOKEN_STRING_LITERAL;

}

;									{ return TOKEN_SEMICOLON; }
,									{ return TOKEN_COMMA; }
:									{ return TOKEN_COLON; }

.									{ return TOKEN_ERROR; }

%%

int yywrap() { return 1; }

bool string_decode(const char *es, char *s) {

    if (es[0] == '\\') {
        if (es[1] != '"') {
            fprintf(stderr, "Encoder: String does not begin with \"\n");
            return false;
        }
    } else if (es[0] != '"') {
        fprintf(stderr, "Encoder: String does not begin with \"\n");
        return false;
    }
    es++;

    if (!*es) {
        fprintf(stderr, "Encoder: String does not end with \"\n");
        return false;
    }

    size_t string_length = 0;
    char decoded_char;

    // march through string until hit closing quotes
    for (; *es; es++) {

        decoded_char = *es;

        if (32 > (int)*es || (int)*es > 127) {
            fprintf(stderr, "Encoder: String contains ASCII values outside the legal range (32-127) \n");
            return false;
        }
    
        // 4. Handle escape sequences
        if (*es == '\\') {
            char highChar, lowChar;
            int high, low;

            // Maybe replace this with a hash table to make it faster
            switch (*(++es)) {
                case 'a':
                    decoded_char = '\a'; // bell
                    break;
                case 'b':
                    decoded_char = '\b'; // backspace
                    break;
                case 'e':
                    decoded_char = '\e'; // escape
                    break;
                case 'f':
                    decoded_char = '\f'; // form feed (clear)
                    break;
                case 'n':
                    decoded_char = '\n'; // newline
                    break;
                case 'r':
                    decoded_char = '\r'; // carriage return
                    break;
                case 't':
                    decoded_char = '\t'; // tab
                    break;
                case 'v':
                    decoded_char = '\v'; // verticle tab
                    break;
                case '\\':
                    decoded_char = '\\'; // backslash
                    break;
                case '\'':
                    decoded_char = '\''; // single quote
                    break;
                case '"':
                    decoded_char = '"'; // double quote
                    break;
                case '0':
                    //check if this is an escaped '0' or a hex sequence
                    if (*(++es) != 'x') {
                        decoded_char = '0';
                        es--;
                        break;
                    }

                    // read in the high and low nibbles of the hex sequence 
                    highChar = *(++es);
                    lowChar = *(++es);

                    // check if high nibble is valid hex digit, convert to number
                    if ( (highChar <= '9' && highChar >= '0') ) {
                        high = highChar - '0';
                    } else if (highChar <= 'f' && highChar >= 'a') {
                        high = highChar - 'a' + 10;
                    } else if (highChar <= 'F' && highChar >= 'A') {
                        high = highChar - 'A' + 10;
                    } else {
                        // return 0 as the character, rescan these tokens.
                        decoded_char = '0';
                        es -= 3;
                        break;
                    }
                    
                    // check if low nibble is valid hex digit, convert to number
                    if ( (lowChar <= '9' && lowChar >= '0') ) {
                        low = lowChar - '0';
                    } else if (lowChar <= 'f' && lowChar >= 'a') {
                        low = lowChar - 'a' + 10;
                    } else if (lowChar <= 'F' && lowChar >= 'A') {
                        low = lowChar - 'A' + 10;
                    } else {
                        // return 0 as the character, rescan these tokens.
                        decoded_char = '0';
                        es -= 3;
                        break;
                    }

                    // interpret nibbles as 2 digit hex humber 
                    decoded_char = (high * 16) + low;
                    break;

                default:
                    decoded_char = *es;
            }

        }
        
        // write this character to s
        *(s + string_length) = decoded_char;
        string_length++;

        // Allow an extra character 255 + 1 because the closing double quotes which is stripped after this loop
        if (string_length > 256) {
            fprintf(stderr, "Encoder: String has length greater than 255\n");
            return false;
        }

    }

    // 3. String must end with "
    if (*(es - 1) != '"') {
        fprintf(stderr, "Encoder: String does not end with \"\n");
        return false;
    }
    
    *(s + string_length - 1) = '\0';
    
    return true;
}
