#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    uint16_t len;
    uint16_t* dial;
    uint16_t* pos;
    uint64_t crossed_zero_count; // for part 2
} safe_t;

safe_t* make_safe(uint16_t dial_max, uint16_t starting_val) {
    safe_t* safe = (safe_t*)malloc(sizeof(safe_t));
    safe->dial = (uint16_t*)malloc(sizeof(uint16_t) * (dial_max) + 1);
    safe->len = dial_max + 1;
    safe->crossed_zero_count = 0;
    for (uint16_t i = 0; i <= dial_max; i++) {
        safe->dial[i] = i;
        if (i == starting_val)
            safe->pos = &safe->dial[i];
    }
    return safe;
}

void reset_safe(safe_t* safe) {
    safe->pos = safe->dial + 50;
    safe->crossed_zero_count = 0;
}

uint16_t safe_dial_value(safe_t* safe) {
    return *safe->pos;
}

uint16_t turn_dial_left(safe_t* safe, uint16_t count) {
    while (count > 0) {
        if (safe->pos == safe->dial) {
            safe->pos += safe->len - 1;
            count--;
        } else {
            safe->pos--;
            count--;
        }

        if (safe_dial_value(safe) == 0 && count > 0) {
            safe->crossed_zero_count++;
        }
    }

    return safe_dial_value(safe);
}

uint16_t turn_dial_right(safe_t* safe, uint16_t count) {
    while (count > 0) {
        if (safe->pos == safe->dial + safe->len - 1) {
            safe->pos = safe->dial;
            count--;
        } else {
            safe->pos++;
            count--;
        }

        if (safe_dial_value(safe) == 0 && count > 0) {
            safe->crossed_zero_count++;
        }
    }
    return safe_dial_value(safe);
}

void test_safe() {
    safe_t* safe = make_safe(99, 50);
    int password = 0;
    assert(turn_dial_left(safe, 68) == 82);
    assert(safe->crossed_zero_count == 1);
    assert(turn_dial_left(safe, 30) == 52);
    assert(turn_dial_right(safe, 48) == 0);
    password++;
    assert(turn_dial_left(safe, 5) == 95);
    assert(turn_dial_right(safe, 60) == 55);
    printf("Got %i, expected %i\n", safe->crossed_zero_count , 2);
    assert(safe->crossed_zero_count == 2);
    assert(turn_dial_left(safe, 55) == 0);
    password++;
    assert(turn_dial_left(safe, 1) == 99);
    assert(turn_dial_left(safe, 99) == 0);
    password++;
    assert(turn_dial_right(safe, 14) == 14);
    assert(turn_dial_left(safe, 82) == 32);
    assert(password == 3);

    reset_safe(safe);
    assert(safe_dial_value(safe) == 50);

    assert(turn_dial_left(safe, 500) == 50);
}

int main() {
    test_safe();
    printf("Test passed.\n");
    safe_t* safe = make_safe(99, 50);

    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE");
        exit(-1);
    }

    char buffer[64];
    int password = 0;
    uint64_t max = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (buffer[0] == 'L') {
            int count = atoi(buffer + 1);
            if (count > max)
                max = count;
            turn_dial_left(safe, count);
        } else if (buffer[0] == 'R') {
            int count = atoi(buffer + 1);
            if (count > max)
                max = count;
            turn_dial_right(safe, count);
        } else {
            printf("SOMETHING WENT WRONG WHILE READING THE FILE.");
            exit(-1);
        }

        // if (password == 0)
        //     printf("(%i) %s", safe_dial_value(safe), buffer);
        if (safe_dial_value(safe) == 0) {
            password++;
            printf("(%i) %s", safe_dial_value(safe), buffer);
        }
    }
    printf("max turns in one rotation: %i\n", max);
    printf("part1: %i\n", password);
    printf("part2: %i\n", safe->crossed_zero_count + password);

    fclose(file);
}
