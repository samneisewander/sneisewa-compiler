#include "scratch.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define SCRATCH_REGISTERS 7

/**
 * The scratch register free list. The indices into the array are the scratch registers. The boolean value at each index indicates if the register is in use (true) or not (false).
 */
bool scratch_free_list[SCRATCH_REGISTERS] = {0};

int scratch_alloc() {
    for (int i = 0; i < SCRATCH_REGISTERS; i++) {
        if (!scratch_free_list[i]) {
            scratch_free_list[i] = true;
            return i;
        }
    }
    fprintf(stderr, "codegen: ran out of scratch registers.\n");
    exit(EXIT_FAILURE);
}

void scratch_free(int r) {
    if (r < 0 || r >= SCRATCH_REGISTERS) return;
    scratch_free_list[r] = false;
}

const char* scratch_name(int r) {
    switch (r) {
        case 0:
            return "%rbx";
        case 1:
            return "%r10";
        case 2:
            return "%r11";
        case 3:
            return "%r12";
        case 4:
            return "%r13";
        case 5:
            return "%r14";
        case 6:
            return "%r15";
        default:
            return "%r15";
    }
}