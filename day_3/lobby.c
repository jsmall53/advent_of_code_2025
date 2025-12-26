#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

/* Largest integer formed by 2 digits in a number 
 * - Order matters, cant look backwards.
 * - can probably use a table with 9 entries to keep
 *   track. This will eliminate duplicate processing
 *   Store the largest number seen for each digit. Then
 *   it will be easy to walk the table to find the largest 
 *   number.
 *
 *
 * Answer: Sum of max joltage from each bank.
 * */

typedef struct {
    int8_t* table;
} digit_table_t;

void digit_table_reset(digit_table_t* table) {
    for (int i = 0; i < 9; i++) {
        table->table[i] = -1;
    }
}

void digit_table_print(digit_table_t* table) {
    for (int i = 0; i < 9; i++) {
        printf("%i: %i\n", i + 1, table->table[i]);
    }
}

int8_t digit_table_get_value(digit_table_t* table, int8_t digit) {
    if (digit < 1 || digit > 9)
        return -1;
    return table->table[digit - 1];
}

void digit_table_set_value(digit_table_t* table, int8_t digit, int8_t val) {
    table->table[digit - 1] = val;
}

int8_t digit_table_get_max(digit_table_t* table) {
    int8_t max = -1;
    for (int i = 0; i < 9; i++) {
        int8_t value_digit = table->table[i];
        if (value_digit == -1)
            continue;
        int8_t val = i + 1;
        val = (val * 10) + value_digit;
        if (val > max)
            max = val;
    }
    return max;
}

digit_table_t* digit_table_create() {
    digit_table_t* table = (digit_table_t*)malloc(sizeof(digit_table_t));
    table->table = (int8_t*)malloc(sizeof(int8_t) * 9);
    digit_table_reset(table);
    return table;
}

uint32_t get_max_joltage(const char* bank, digit_table_t* table) {
    digit_table_reset(table);

    while (isdigit(*bank)) {
        int8_t key_digit = *bank - '0';
        if (digit_table_get_value(table, key_digit) != -1) {
            // assume we've already seen the best possible number for this digit.
            // skip
            bank++;
            continue;
        }
        const char* tmp = bank + 1;
        while (isdigit(*tmp)) {
            int8_t val_digit = *tmp - '0';
            if (val_digit > digit_table_get_value(table, key_digit)) {
                digit_table_set_value(table, key_digit, val_digit);
            }
            tmp++;
        }

        bank++;
    }
    return digit_table_get_max(table);
}

void test_table() {
    digit_table_t* table = digit_table_create();
    assert(digit_table_get_value(table, 1) == -1);
    digit_table_set_value(table, 1, 8);
    assert(digit_table_get_value(table, 1) == 8);
    printf("%i\n", digit_table_get_max(table));
    assert(digit_table_get_max(table) == 18);
    digit_table_set_value(table, 8, 9);
    assert(digit_table_get_max(table) == 89);
    digit_table_set_value(table, 9, 1);
    assert(digit_table_get_max(table) == 91);


    printf("test_table passed.\n");
}

void test_bank() {
    digit_table_t* table = digit_table_create();
    printf("%d\n", get_max_joltage("987654321111111", table));
    assert(get_max_joltage("987654321111111", table) == 98);
    assert(get_max_joltage("811111111111119", table) == 89);
    assert(get_max_joltage("234234234234278", table) == 78);
    assert(get_max_joltage("818181911112111", table) == 92);

    printf("test_bank passed.\n");
}

int main() {
    test_table();
    test_bank();
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE.");
        exit(-1);
    }

    char buf[128];
    uint64_t total_joltage = 0;
    digit_table_t* table = digit_table_create();
    while (fgets(buf, sizeof(buf), file) != NULL) {
        int len = strlen((const char*)buf);
        uint32_t joltage = get_max_joltage(buf, table);
        if (joltage > 127) {
            printf("ERROR: got max joltage of %d\n", joltage);
            digit_table_print(table);
            printf("%s (%i)\n", buf);
            assert(false);
        }
        total_joltage += joltage;
    }

    printf("Total joltage: %zu\n", total_joltage);
}
