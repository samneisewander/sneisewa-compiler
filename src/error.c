/* error.c */

#include "error.h"

#include <stdbool.h>

bool Error = false;

void set_error(bool error) {
    Error = error;
}

bool check_error() {
    return Error;
}