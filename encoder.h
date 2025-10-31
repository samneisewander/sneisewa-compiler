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

#endif