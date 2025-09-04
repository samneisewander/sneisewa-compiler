#include <stdio.h>

// use od program to see these as raw bytes

int main() {
    FILE *out = fopen("./out.bminor", "w+");

    if (!out) {
        return 1;
    }

    fprintf(out, "\"%c <- that is a terrible character\"", 127);

    fclose(out);

    return 0;
}