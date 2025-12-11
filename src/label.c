#include "label.h"

#include <stdio.h>
#include <stdlib.h>

#define LABEL_MAX_LENGTH 32
int label_counter = 0;

int label_create() {
    return label_counter++;
}

const char* label_name(int label) {
    static char label_str[LABEL_MAX_LENGTH] = {0};
    snprintf(label_str, LABEL_MAX_LENGTH, ".L%d", label);
    return label_str;
}