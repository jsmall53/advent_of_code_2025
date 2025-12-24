#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct {
    char* begin;
    char* end;
} id_range_t;

void id_range_print(id_range_t* range) {
    int n = range->end - range->begin + 1;
    printf("%.*s\n", n, range->begin); 
}

uint64_t id_range_first(id_range_t* range) {
    char* c = range->begin;

    uint64_t accum = 0;
    while (*c != '-') {
        int val = *c - '0';
        accum = accum * 10 + val;
        c++;
    }
    return accum;
}

uint64_t id_range_last(id_range_t* range) {
    char* c = range->begin;
    while (*c != '-') c++;
    char* begin = c++;
    uint64_t accum = 0;
    while (c <= range->end) {
        int val = *c - '0';
        accum = accum * 10 + val;
        c++;
    }
    return accum;
}

int main() {

    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE.");
        exit(-1);
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        printf("FAILED TO READ FILE INTO BUFFER.");
        exit(-2);
    }

    char* pos = buffer;
    char* range_begin = buffer;
    while (*pos != '\0') {
        printf("%c", *pos);
        if (*pos == ',') {
            id_range_t range = { 
                .begin = range_begin,
                .end = pos - 1
            };
            printf("%d-%d,", id_range_first(&range), id_range_last(&range));
            id_range_print(&range);
            pos++;
            range_begin = pos;
        } else {
            pos++;
        }
    }
}
