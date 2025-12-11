#ifndef SCRATCH_H
#define SCRATCH_H

/**
 * Allocates a scratch register from the free list. If none are available, the program will exit.
 */
int scratch_alloc();

/**
 * Frees a scratch register by placing it on the free list, regardless of whether or not it was actually allocated.
 */
void scratch_free(int r);

/**
 * Returns a string containing the x86-64 GNU assembly name corresponding to the scratch register r. If r does not have a defined mapping, "%%r15" is returned.
 */
const char* scratch_name(int r);

#endif