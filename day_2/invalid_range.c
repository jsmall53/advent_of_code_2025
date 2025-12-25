#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


typedef struct {
    const char* begin;
    const char* end;
} id_range_t;

void id_range_print(id_range_t* range) {
    int n = range->end - range->begin + 1;
    printf("%.*s\n", n, range->begin); 
}

uint64_t id_range_first(id_range_t* range) {
    char* c = range->begin;
    uint64_t accum = 0;
    while (*c != '-') {
        int digit = *c - '0';
        accum = accum * 10 + digit;
        c++;
    }
    return accum;
}

uint64_t id_range_last(id_range_t* range) {
    char* c = range->begin;
    while (*c != '-') c++;
    char* begin = c++;
    uint64_t accum = 0;
    while (*c != '\0' && c <= range->end) {
        int val = *c - '0';
        accum = accum * 10 + val;
        c++;
    }
    return accum;
}

void int_to_str(uint64_t num, char* buf, int buf_len) {
    assert(buf != NULL);
    snprintf(buf, buf_len - 1, "%zu", num);
}

#define BUF_LEN 32
bool id_is_invalid(uint64_t id) {
    // int pow = 10;
    // while (id / pow > 0) {
    //     uint64_t temp = id / pow;
    //     // printf("%zu, %zu (%d)\n", id, temp, pow);
    //     if (id % (temp * pow) == temp) {
    //         return true;
    //     }
    //     pow *= 10;
    // }
    char buf[BUF_LEN];
    int_to_str(id, buf, BUF_LEN);
    int len = strnlen((const char*)buf, BUF_LEN);
    if (len % 2 == 0) {
        char* half_ptr = buf + (len / 2);
        for (size_t i = 0; i < len / 2; i++) {
            if (*(buf + i) != *(half_ptr + i)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

uint64_t id_range_find_invalid(id_range_t* range) {
    uint64_t first = id_range_first(range);
    uint64_t last = id_range_last(range);
    uint64_t cur = first;
    uint64_t accum = 0;
    while (cur <= last) {
        if (id_is_invalid(cur)) {
            printf("Invalid id: %zu\n", cur);
            accum += cur;
        }
        cur++;
    }
    return accum;
}


void test_parse() {
    const char* range_str = "8989806846-8989985017";
    int len = strlen(range_str);
    id_range_t range = {
        .begin = range_str,
        .end = range_str + len - 1
    };
    uint64_t first = id_range_first(&range);
    assert(first == 8989806846);
    uint64_t last = id_range_last(&range);
    printf("test_range last: %zu\n", last);
    assert(last == 8989985017);

    range_str = "35-54";
    len = strlen(range_str);
    range = (id_range_t){ .begin = range_str, .end = range_str + len - 1 };
    first = id_range_first(&range);
    assert(first == 35);
    last = id_range_last(&range);
    assert(last == 54);

}

void test_invalid_ids() {
    assert(id_is_invalid(100100));
    assert(!id_is_invalid(1234567890));
    assert(id_is_invalid(55));
    assert(id_is_invalid(1188511885));
    assert(!id_is_invalid(94000094));
}

int main() {
    test_parse();
    printf("test_parse succeeded.\n");
    test_invalid_ids();
    printf("test_invalid_ids succeeded.\n");
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE.");
        exit(-1);
    }

    char buffer[1024] = {0};
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        printf("FAILED TO READ FILE INTO BUFFER.");
        exit(-2);
    }

    char* pos = buffer;
    char* range_begin = buffer;
    bool end = false;
    uint64_t answer = 0;
    while (pos != NULL && !end) {
        // printf("%c", *pos);
        if (*pos == ',' || *pos == '\0' || isspace(*pos)) {
            id_range_t range = { 
                .begin = range_begin,
                .end = pos - 1
            };

            answer += id_range_find_invalid(&range);
            // printf("%zu\n", answer);

            pos++;
            range_begin = pos;
            if (*pos == '\0') {
                end = true;
            }
        } else {
            pos++;
        }
    } 
    printf("Final answer: %zu\n", answer);
}
