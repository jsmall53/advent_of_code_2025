#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

void part1_read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FAILED TO READ INPUT FILE\n");
        exit(1);
    }

    fclose(file);
}
void part1() {
    part1_read_file("sample.txt");

}

int main() {
    part1();
}
