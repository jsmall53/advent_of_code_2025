#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

typedef enum {
    OP_NONE,
    OP_ADD,
    OP_MUL,
} operation;

#define DEFAULT_LEN 8

typedef struct {
    size_t nums[DEFAULT_LEN];
    size_t len;
    operation op;
} column_t;

void column_debug_print(column_t* col) {
    for (int i = 0; i < col->len; i++) {
        printf("%zu ", col->nums[i]);
    }

    char op_char = '?';
    switch (col->op) {
        case OP_ADD:
            op_char = '+';
            break;
        case OP_MUL:
            op_char = '*';
            break;
        default:
            op_char = '?';
            break;
    }
    printf("%c\n", op_char);
}

void column_add_value(column_t* col, size_t val) {
    col->nums[col->len++] = val;
}

size_t column_calculate(column_t* col) {
    switch (col->op) {
        case OP_ADD: 
        {
            size_t accum = 0;
            for (int i = 0; i < col->len; i++) {
                accum += col->nums[i];
            }
            return accum;
        }
        case OP_MUL:
        {
            size_t accum = 1;
            for (int i = 0; i < col->len; i++) {
                accum *= col->nums[i];
            }
            return accum;
        }
        default:
            printf("ERROR: Invalid column operation.\n");
            assert(false);
            break;
    }
    return 0;
}

void column_reset(column_t* col) {
    memset(col->nums, 0, sizeof(size_t) * DEFAULT_LEN);
    col->len = 0;
    col->op = OP_NONE;
}

typedef struct {
    column_t* columns;
    size_t n_cols;
    size_t cap;
} table_t;

void table_debug_print(table_t* table) {
    for (size_t i = 0; i < table->n_cols; i++) {
        column_debug_print(table->columns + i);
    }
}

table_t* table_create(size_t cap) {
    table_t* table = (table_t*)malloc(sizeof(table_t));
    table->columns = (column_t*)malloc(sizeof(column_t) * cap);
    table->n_cols  = cap;
    return table;
}

void table_free(table_t* table) {
    free(table->columns);
    table->columns = NULL;
    table->n_cols = 0;
    free(table);
}

void table_column_add_value(table_t* table, size_t col, size_t value) {
    column_t* column = &table->columns[col];
    column_add_value(column, value);
}

void table_column_set_op(table_t* table, size_t col, operation op) {
    table->columns[col].op = op;
}

size_t table_calculate(table_t* table) {
    size_t accum = 0;
    for (size_t i = 0; i < table->n_cols; i++) {
        accum += column_calculate(table->columns + i);
    }
    return accum;
}

table_t* read_file_into_table(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE: %s\n", filename);
        exit(1);
    }

    char buf[4096];
    table_t* table = NULL;
    while (fgets(buf, sizeof(buf), file) != NULL) {
        if (table == NULL) {
            // 1) read the buffer to get the number of columns;
            size_t current = 0;
            char* c = buf;
            size_t n_cols = 0;
            while (*c != '\0' && *c != 0) {
                if (isspace(*c)) {
                    // add this number to the next column if > 0
                    if (current > 0)
                        n_cols++;
                    current = 0;
                } else if (isdigit(*c)) {
                    int digit = *c - '0';
                    current = current * 10 + digit;
                }
                c++;
            }
            // 2) initialize the table
            table = table_create(n_cols);
        }

        size_t val = 0;
        size_t col = 0;
        char* c = buf;
        while (*c != '\0' && *c != 0) {
            if (isspace(*c)) {
                if (val > 0) {
                    table_column_add_value(table, col++, val);
                }
                val = 0;
                if (*c == '\n') {
                    col = 0;
                }
            } else if (isdigit(*c)) {
                int digit = *c - '0';
                val = val * 10 + digit;
            } else {
                switch (*c) {
                    case '+':
                        table_column_set_op(table, col++, OP_ADD);
                        break;
                    case '*':
                        table_column_set_op(table, col++, OP_MUL);
                        break;
                    default:
                        printf("ERROR: UNSUPPORTED TOKEN\n");
                        assert(false);
                }
            }
            c++;
        }
    }
    fclose(file);
    return table;
}

void table_check_cols(table_t* table) {
    for (int i = 0; i < table->n_cols; i++) {
        column_t* col = &table->columns[i];
        if (col->len <= 0) {
            printf("Col %i len=%zu\n", i, col->len);
            assert(col->len > 0);
        }
        assert(table->columns[i].op != OP_NONE);
    }
}

void part1() {
    table_t* table = read_file_into_table("sample.txt");
    table_check_cols(table);
    assert(table_calculate(table) == 4277556);
    table_free(table);
    table = NULL;
    printf("part 1 sample case passed.\n");

    table = read_file_into_table("input.txt");
    table_check_cols(table);
    size_t ans = table_calculate(table);
    // 14379972759 is too low
    printf("Part 1 result: %zu\n", ans);
}

int main() {
    part1();
}
