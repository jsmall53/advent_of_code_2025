#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    size_t start;
    size_t end;
} range_t;

bool range_contains(range_t* range, size_t n) {
    if (n >= range->start && n <= range->end)
        return true;
    return false;
}

typedef struct {
    range_t* ptr;
    size_t cap;
    size_t len;
} range_array_t;

range_array_t* range_array_create(size_t cap) {
    range_array_t* arr = (range_array_t*)malloc(sizeof(range_array_t));
    arr->len = 0;
    arr->ptr = (range_t*)malloc(sizeof(range_t) * cap);
    arr->cap = cap;
    return arr;
}

range_t* range_array_add_range(range_array_t* arr, size_t start, size_t end) {
    if (arr->len >= arr->cap) {
        size_t new_cap = arr->cap * 2;
        arr->ptr = (range_t*)realloc(arr->ptr, new_cap * sizeof(range_t));
        arr->cap = new_cap;
    }

    range_t* range = &arr->ptr[arr->len++];
    range->start   = start;
    range->end     = end;
    return range;
}

int find_char(const char* buf, char c) {
    int offset = 0;
    while (*(buf + offset) != '\0' && !isspace(*(buf + offset))) {
        if (*(buf + offset) == c)
            return offset;
        offset++;
    }
    offset = -1;
    printf("\n");
    return offset;
}

size_t string_to_id(const char* buf, int start, int end) {
    size_t accum = 0;
    for (int i = start; i < end; i++) {
        int digit = buf[i] - '0';
        accum = accum * 10 + digit;
    }
    return accum;
}

int main() {
    assert(find_char("123-456", '-') == 3);
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO OPEN INPUT FILE.\n");
        exit(-1);
    }

    char buf[256];
    size_t line = 0;
    bool check_freshness = false;
    range_array_t* range_array = range_array_create(150);
    size_t fresh = 0;
    while(fgets(buf, sizeof(buf), file) != NULL) {
        line += 1;
        if (isspace(*buf)) {
            check_freshness = true;
        } else if (check_freshness) {
            // check freshness of single id
            assert(range_array->len == 182);
            size_t id = string_to_id(buf, 0, strlen(buf) - 1);
            for (int i = 0; i < range_array->len; i++) {
                if (range_contains(range_array->ptr + i, id)) {
                    fresh += 1;
                    break;
                }
            }
        } else { // parse id range
            int split_idx = find_char(buf, '-');
            assert(split_idx != -1);
            size_t start = string_to_id(buf, 0, split_idx);
            size_t end = string_to_id(buf, split_idx + 1, strlen(buf) - 1);
            range_array_add_range(range_array, start, end);
        }
    }

    printf("Part 1: %zu\n", fresh);
}
