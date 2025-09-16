%{
	#include "tokens.h"
%}

DIGIT 				[0-9]
NONZERO_DIGIT 		[1-9]
HEX					[0-9a-fA-F]
NONZERO_HEX			[1-9a-fA-F]
LETTER 				[a-zA-Z]
ALPHANUM 			[a-zA-Z0-9]

IDENTIFIER 			({LETTER}|_)(ALPHANUM|_){0,254}

ESCAPE_SEQUENCE   	\\[abefrntv\\\'\"]
SYMBOL				[ !\"\#$\%&'\(\)\*\+,\-\.\/:;<=>?@\[\]\^_`\{|\}~]

CHARACTER			({ALPHANUM}|{ESCAPE_SEQUENCE}|{SYMBOL})

INTEGER_LITERAL 	{NONZERO_DIGIT}{DIGIT}*
HEX_LITERAL 		{NONZERO_HEX}{HEX}*H
BINARY_LITERAL 		1[01]*B

%%

(" "|\t|\n)         			/* skip whitespace */
0        						/* skip leading zeroes */
\(  							{ return TOKEN_LPAREN; }   
\)    							{ return TOKEN_RPAREN; }   
\[    							{ return TOKEN_LBRACKET; }   
\]   							{ return TOKEN_RBRACKET; }   
\{    							{ return TOKEN_LCURLY; }   
\}    							{ return TOKEN_RCURLY; }   

\+    							{ return TOKEN_ADD; }      
\-   							{ return TOKEN_SUBTRACT; }      
\*    							{ return TOKEN_MULTIPLY; }     
\\    							{ return TOKEN_DIVIDE; }      
\^    							{ return TOKEN_EXPONENT; }      
\%    							{ return TOKEN_REMAINDER; }      

"++"							{ return TOKEN_POSTFIX_INCREMENT; }       
"--"   							{ return TOKEN_POSTFIX_DECREMENT; }       

"#"    							{ return TOKEN_UNARY_ARRAY_LENGTH; }   

"<"    							{ return TOKEN_LESS_THAN; }       
"<="   							{ return TOKEN_LESS_THAN_OR_EQUAL; }      
"=="    						{ return TOKEN_EQUAL; }       
">"    							{ return TOKEN_GREATER_THAN; }       
">="    						{ return TOKEN_GREATER_THAN_OR_EQUAL; }       
"!="    						{ return TOKEN_NOT_EQUAL; }       

"!"    							{ return TOKEN_LOGICAL_NOT; }      
"&&"   							{ return TOKEN_LOGICAL_AND; }      
"||"   							{ return TOKEN_LOGICAL_OR; }       

"="    							{ return TOKEN_ASSIGNMENT; }

array    						{ return TOKEN_KEY_ARRAY; }
auto    						{ return TOKEN_KEY_AUTO; }
boolean    						{ return TOKEN_KEY_BOOLEAN; }
char    						{ return TOKEN_KEY_CHAR; }
else    						{ return TOKEN_KEY_ELSE; }
false    						{ return TOKEN_KEY_FALSE; }
float    						{ return TOKEN_KEY_FLOAT; }
double    						{ return TOKEN_KEY_DOUBLE; }
for    							{ return TOKEN_KEY_FOR; }
function  						{ return TOKEN_KEY_FUNCTION; }
if    							{ return TOKEN_KEY_IF; }
integer    						{ return TOKEN_KEY_INTEGER; }
print    						{ return TOKEN_KEY_PRINT; }
return    						{ return TOKEN_KEY_RETURN; }
string    						{ return TOKEN_KEY_STRING; }
true    						{ return TOKEN_KEY_TRUE; }
void    						{ return TOKEN_KEY_VOID; }
while    						{ return TOKEN_KEY_WHILE; }

{IDENTIFIER} 					{ return TOKEN_IDENTIFIER; }

{NONZERO_DIGIT}{DIGIT}*	  								{ return TOKEN_INTEGER_LITERAL; }
{NONZERO_DIGIT}{DIGIT}*\.{DIGIT}*{NONZERO_DIGIT}    	{ return TOKEN_DOUBLE_LITERAL; }
'{CHARACTER}'    										{ return TOKEN_CHARACTER_LITERAL; }
\"{CHARACTER}*\"    									{ return TOKEN_STRING_LITERAL; }

;								{ return TOKEN_SEMICOLON; }
,								{ return TOKEN_COMMA; }

.								{ return TOKEN_ERROR; }

%%

int yywrap() { return 1; } 
