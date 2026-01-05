#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define BUFFER_SIZE     1024

#define CHAR_INIT_BEAM  'S'
#define CHAR_BEAM       '|'
#define CHAR_SPLIT      '^'

size_t str_count_beams(const char* buf) {
    size_t count = 0;
    for (size_t i = 0; i < strlen(buf); i++) {
        if (buf[i] == CHAR_BEAM)
            count++;
    }
    return count;
}


size_t part1_read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FAILED TO READ INPUT FILE\n");
        exit(1);
    }

    char buf[BUFFER_SIZE] = {0};
    char buf_prev[BUFFER_SIZE] = {0};
    size_t split_count = 0;
    while (fgets(buf, sizeof(buf), file) != NULL) {
        for (size_t i = 0; i < strlen(buf); i++) {
            char c = buf[i];
            if (isspace(c))
                continue;

            switch (c) {
                case '.':
                    if (buf_prev[i] == CHAR_BEAM) {
                        buf[i] = CHAR_BEAM;
                    }
                    break;
                case CHAR_SPLIT:
                    // assume no splits are directly 
                    // next to each other.
                    if (buf_prev[i] == CHAR_BEAM) {
                        if (i > 0) 
                            buf[i - 1] = CHAR_BEAM;
                        if (i < strlen(buf) - 1)
                            buf[i + 1] = CHAR_BEAM;
                        split_count++;
                    }
                    break;
                case CHAR_INIT_BEAM:
                    buf[i] = CHAR_BEAM;
                    break;
                case CHAR_BEAM:
                    // happens if split is proccessed
                    // on previous char. just skip.
                    // Assuming splits aren't next to each
                    // other this is safe.
                    break;
                default:
                    printf("Unhandled token: '%c'\n", c);
                    assert(false);
                    break;
            }
        }
        memcpy(buf_prev, buf, BUFFER_SIZE);
    }
    fclose(file);

    return split_count;
}
void part1() {
    assert(part1_read_file("sample.txt") == 21);
    printf("part1 sample passed.\n");
    printf("part1 result: %zu\n", part1_read_file("input.txt"));
}

int main() {
    part1();
}
