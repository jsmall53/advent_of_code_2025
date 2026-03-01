#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>

#define STRIDE 3
#define N_PRESENTS 6
#define MAX_REGIONS 1000

#define DEBUG

typedef struct {
    int width;
    int height;
    int counts[N_PRESENTS];
} Region;

typedef struct {
    uint16_t n_presents;
    uint16_t presents[N_PRESENTS];
    uint16_t n_regions;
    Region regions[MAX_REGIONS];
} State;

#define BUF_SIZE 256
void read_file(const char* filename, State* state) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(-1);
    }

    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, file)) {
        const char* ptr = buf;
        while (*ptr != '\0' && ptr != 0) {
            if (isspace(*ptr)) {
                ptr++;
                continue;
            }

            if (isdigit(*ptr)) {
                int value = 0;
                while (isdigit(*ptr)) {
                    value = value * 10 + (*ptr - '0');
                    ptr++;
                }

                if (*ptr == ':') {
                    // expect a present type
                    assert(*ptr == ':');
                    ptr++;
                    state->n_presents++;
                } else if (*ptr == 'x') {
                    // expect a tree region
                    assert(state->n_regions < MAX_REGIONS);
                    Region* region = &state->regions[state->n_regions++];
                    region->width = value;
                    ptr++;
                    assert(isdigit(*ptr));

                    int height = 0;
                    while (isdigit(*ptr)) {
                        height = height * 10 + (*ptr - '0');
                        ptr++;
                    }
                    region->height = height;
                    assert(*ptr == ':');
                    ptr++;
                    while (isspace(*ptr)) {
                        ptr++;
                    }


                    int region_idx = 0;
                    while (*ptr != '\n' && region_idx < N_PRESENTS) {
                        if (isspace(*ptr)) {
                            region_idx++;
                            ptr++;
                            continue;
                        } else if (isdigit(*ptr)) {
                            region->counts[region_idx] = region->counts[region_idx] * 10 + (*ptr - '0');
                            ptr++;
                        } else {
                            fprintf(stderr, "Unexpected token '%c' while parsing region\n", *ptr);
                            assert(0);
                        }
                    }

                }
            } else if (*ptr == '#') {
                state->presents[state->n_presents - 1] <<= 1;
                state->presents[state->n_presents - 1] |= 1;
                ptr++;
            } else if (*ptr == '.') {
                state->presents[state->n_presents - 1] <<= 1;
                ptr++;
            } else {
                fprintf(stderr, "Unexpected token '%c'\n", *ptr);
                assert(0);
            }
        }
    }

#ifdef DEBUG
    for (int i = 0; i < state->n_presents; i++) {
        fprintf(stdout, "present %i: %X\n", i, state->presents[i]);
    }

    for (int i = 0; i < state->n_regions; i++) {
        Region* region = &state->regions[i];
        fprintf(stdout, "region %i, %ix%i: ", i, region->width, region->height);
        for (int k = 0; k < N_PRESENTS; k++) {
            fprintf(stdout, "%i ", region->counts[k]);
        }
        fprintf(stdout, "\n");
    }
#endif
}

void part1(const char* filename) {
    State state = {0};
    read_file(filename, &state);
}

int main() {
    fprintf(stderr, "farm\n");
    part1("sample.txt");
    part1("input.txt");
}
