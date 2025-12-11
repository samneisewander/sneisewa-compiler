#ifndef LABEL_H
#define LABEL_H

/**
 * Creates a new, unique, anonymous
label that can be used to indicate the target of jumps and conditional branch instructions.
 */
int label_create();

/**
 * Returns the string representation of an assembly label created using label_create.
 */
const char* label_name(int label);

#endif