#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool string_decode(const char *es, char *s);
void string_encode(const char *s, char *es);

int main(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--encode") == 0) {
            
            // open passed file, allocate buffers for string encoding/decoding
            FILE *fp = fopen(argv[i + 1], "r");
            char encoded[BUFSIZ];
            char decoded[BUFSIZ];
            
            // read in the string, flose file pointer
            fgets(encoded, BUFSIZ, fp);
            fclose(fp);

            // test string encoding/decoding
            if (string_decode(encoded, decoded)) {
                string_encode(decoded, encoded);
                printf("Encoded string: %s\n", encoded);
                
                return EXIT_SUCCESS;
            }
            else {
                return EXIT_FAILURE;
            }
        }
    }
}

bool string_decode(const char *es, char *s) {

    if (*es == '\\') {
        if (*(++es) != '"') {
            fprintf(stderr, "Encoder: String does not begin with \"\n");
            return false;
        }
    } else if (*(es++) != '"') {
        fprintf(stderr, "Encoder: String does not begin with \"\n");
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
                        es -= 1;
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
    printf("Encoder: Valid string of length %ld: %s\n", string_length, s);
    
    return true;
}

void string_encode(const char *s, char *es) {

    *(es++) = '"';

    // the first statement of a for loop is an initializer. if you dont need to initialize the
    // control variable, you can just leave that blank.
    // replace this with a hashtable to make it faster
    for (; *s; s++) {
        switch (*s) {
            case '\a':
                *(es++) = '\\'; // bell
                *es = 'a';
                break;
            case '\b':
                *(es++) = '\\'; // backspace
                *es = 'b';
                break;
            case '\e':
                *(es++) = '\\'; // escape
                *es = 'e';
                break;
            case '\f':
                *(es++) = '\\'; // form feed
                *es = 'f';
                break;
            case '\n':
                *(es++) = '\\'; // newline
                *es = 'n';
                break;
            case '\r':
                *(es++) = '\\'; // carriage return
                *es = 'r';
                break;
            case '\t':
                *(es++) = '\\'; // tab
                *es = 't';
                break;
            case '\v':
                *(es++) = '\\'; // vertical tab
                *es = 'v';
                break;
            case '\\':
                *(es++) = '\\'; // backslash
                *es = '\\';
                break;
            case '\'':
                *(es++) = '\\'; // single quote
                *es = '\'';
                break;
            case '"':
                *(es++) = '\\'; // double quote
                *es = '"';
                break;
            default:
                *es = *s;
                break;
        }
        es++;
    }

    *(es++) = '"'; // ending quote
    *es = '\0'; // null terminator
}