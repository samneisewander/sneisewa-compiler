#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

extern FILE *yyin;
extern int yylex();
extern char *yytext;

const char *token_t_map[] = {
    "TOKEN_EOF",

    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACKET",
    "TOKEN_RBRACKET",
    "TOKEN_LCURLY",
    "TOKEN_RCURLY",

    "TOKEN_ADD",
    "TOKEN_SUBTRACT",
    "TOKEN_MULTIPLY",
    "TOKEN_DIVIDE",
    "TOKEN_EXPONENT",
    "TOKEN_REMAINDER",

    "TOKEN_POSTFIX_INCREMENT",
    "TOKEN_POSTFIX_DECREMENT",

    "TOKEN_UNARY_ARRAY_LENGTH",
    "TOKEN_UNARY_NEGATE",

    "TOKEN_LESS_THAN",
    "TOKEN_LESS_THAN_OR_EQUAL",
    "TOKEN_EQUAL",
    "TOKEN_GREATER_THAN",
    "TOKEN_GREATER_THAN_OR_EQUAL",
    "TOKEN_NOT_EQUAL",

    "TOKEN_LOGICAL_NOT",
    "TOKEN_LOGICAL_AND",
    "TOKEN_LOGICAL_OR",

    "TOKEN_ASSIGNMENT",

    "TOKEN_KEY_ARRAY",
    "TOKEN_KEY_AUTO",
    "TOKEN_KEY_BOOLEAN",
    "TOKEN_KEY_CHAR",
    "TOKEN_KEY_ELSE",
    "TOKEN_KEY_FALSE",
    "TOKEN_KEY_FLOAT",
    "TOKEN_KEY_DOUBLE",
    "TOKEN_KEY_FOR",
    "TOKEN_KEY_FUNCTION",
    "TOKEN_KEY_IF",
    "TOKEN_KEY_INTEGER",
    "TOKEN_KEY_PRINT",
    "TOKEN_KEY_RETURN",
    "TOKEN_KEY_STRING",
    "TOKEN_KEY_TRUE",
    "TOKEN_KEY_VOID",
    "TOKEN_KEY_WHILE",

    "TOKEN_IDENTIFIER",

    "TOKEN_INTEGER_LITERAL",
    "TOKEN_HEX_LITERAL",
    "TOKEN_BINARY_LITERAL",
    "TOKEN_DOUBLE",
    "TOKEN_DOUBLE_SCIENTIFIC",

    "TOKEN_CHARACTER_LITERAL",
    "TOKEN_STRING_LITERAL",

    "TOKEN_SEMICOLON",
    "TOKEN_COMMA",
    "TOKEN_COLON",

    "TOKEN_ERROR"};

bool string_decode(const char *es, char *s);
void string_encode(const char *s, char *es);

int main(int argc, char *argv[]) {
    char encoded[BUFSIZ];
    char decoded[BUFSIZ];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--encode") == 0) {
            // open passed file, allocate buffers for string encoding/decoding
            FILE *fp = fopen(argv[i + 1], "r");

            // read in the string, flose file pointer
            fgets(encoded, BUFSIZ, fp);
            fclose(fp);

            // test string encoding/decoding
            if (string_decode(encoded, decoded)) {
                string_encode(decoded, encoded);
                printf("Encoded string: %s\n", encoded);

                return EXIT_SUCCESS;
            } else {
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--scan") == 0) {
            yyin = fopen(argv[++i], "r");
            if (!yyin) {
                fprintf(stderr, "Could not open file %s\n", argv[i]);
                return EXIT_FAILURE;
            }

            int status = EXIT_SUCCESS;

            for (token_t t = yylex(); t != TOKEN_EOF; t = yylex()) {
                if (t == TOKEN_ERROR) {
                    status = EXIT_FAILURE;
                    continue;
                }

                // Print the token type and the decoded literal if applicable.
                switch (t) {
                    case TOKEN_IDENTIFIER:
                    case TOKEN_INTEGER_LITERAL:
                    case TOKEN_HEX_LITERAL:
                    case TOKEN_BINARY_LITERAL:
                    case TOKEN_DOUBLE:
                    case TOKEN_DOUBLE_SCIENTIFIC:
                    case TOKEN_CHARACTER_LITERAL:
                    case TOKEN_STRING_LITERAL:
                        printf("%s %s\n", token_t_map[t], yytext);
                        break;
                    default:
                        printf("%s\n", token_t_map[t]);
                        break;
                }
            }

            return status;
        }
    }
}

void string_encode(const char *s, char *es) {
    *(es++) = '"';

    // the first statement of a for loop is an initializer. if you dont need to initialize the
    // control variable, you can just leave that blank.
    // replace this with a hashtable to make it faster
    for (; *s; s++) {
        switch (*s) {
            case '\a':
                *(es++) = '\\';  // bell
                *es = 'a';
                break;
            case '\b':
                *(es++) = '\\';  // backspace
                *es = 'b';
                break;
            case '\e':
                *(es++) = '\\';  // escape
                *es = 'e';
                break;
            case '\f':
                *(es++) = '\\';  // form feed
                *es = 'f';
                break;
            case '\n':
                *(es++) = '\\';  // newline
                *es = 'n';
                break;
            case '\r':
                *(es++) = '\\';  // carriage return
                *es = 'r';
                break;
            case '\t':
                *(es++) = '\\';  // tab
                *es = 't';
                break;
            case '\v':
                *(es++) = '\\';  // vertical tab
                *es = 'v';
                break;
            case '\\':
                *(es++) = '\\';  // backslash
                *es = '\\';
                break;
            case '\'':
                *(es++) = '\\';  // single quote
                *es = '\'';
                break;
            case '"':
                *(es++) = '\\';  // double quote
                *es = '"';
                break;
            default:
                *es = *s;
                break;
        }
        es++;
    }

    *(es++) = '"';  // ending quote
    *es = '\0';     // null terminator
}