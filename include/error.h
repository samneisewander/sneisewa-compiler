/* error.h */

#ifndef ERROR_H
#define ERROR_H
#include <stdbool.h>

// Sets the global error state.
void set_error(bool error);

// Returns the global error state
bool check_error();
#endif