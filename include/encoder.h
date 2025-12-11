#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>

/**
 * Decodes a Bminor string for from is source code value to its literal value.
 *
 * @param es encoded string: input string
 * @param s decoded string: buffer to write encoded string to.
 */
bool string_decode(const char* es, char* s);

/**
 * Encodes a Bminor string for printing.
 *
 * @param s string: input string
 * @param es encoded string: buffer to write encoded string to.
 */
void string_encode(const char* s, char* es);

/**
 * Takes an input character, which may be a control character, and returns a string containing the encoded representation of the character wrapped in single quotes, the way it might appear in a bminor program. The return string may include backtick escape codes like "\\n" or "\t".
 *
 * Examples: Newline => "'\\n'", h => 'h'
 *
 * @param c character to encode
 * @param es Buffer to write encoded character to.
 */
void char_encode(char c, char* es);

#endif