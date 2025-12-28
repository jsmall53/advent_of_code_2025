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

typedef struct {
    char* ptr;
    size_t len;
    size_t cap;
} string_t;

string_t* string_create(size_t capacity) {
    string_t* string = (string_t*)malloc(sizeof(string_t));
    string->ptr = (char*)malloc(sizeof(char) * capacity);
    string->cap = capacity;
    string->len = 0;
    return string;
}

string_t* string_copy(string_t* str) {
    string_t* copy = (string_t*)malloc(sizeof(string_t));
    copy->ptr = (char*)malloc(sizeof(char) * str->cap);
    memcpy(copy->ptr, str->ptr, str->len);
    copy->cap = str->cap;
    copy->len = str->len;
    return copy;
}

size_t string_append_char(string_t* str, char c) {
    if (str->len == str->cap) {
        // @TODO: dynamically resize if needed.
        assert(false);
    }
    str->ptr[str->len++] = c;
    return str->len;
}

const char* string_value_copy(string_t* str) {
    char* ret = (char*)malloc(sizeof(str->len + 1));
    snprintf(ret, str->len + 1, "%s", str->ptr);
    return ret;
}

void string_reverse(string_t* str) {
    char* front = str->ptr;
    char* back = str->ptr + str->len - 1;
    while (back > front) {
        char tmp = *front;
        *front = *back;
        *back = tmp;

        back--;
        front++;
    }
}

uint64_t string_atoi(string_t* str, bool reverse) {
    uint64_t accum = 0;
    if (!reverse) {
        for (size_t i = 0; i < str->len; i++) {
            uint8_t digit = *(str->ptr + i) - '0';
            accum = accum * 10 + digit;
        }
    } else { // reverse
        for (int i = (str->len - 1); i >= 0; i--) {
            uint8_t digit = *(str->ptr + i) - '0';
            accum = accum * 10 + digit;
        }
    }
    return accum;
}

void test_str() {
    string_t* str = string_create(12);
    for (char c = '1'; c <= '9'; c++) {
        string_append_char(str, c);
    }
    assert(str->len == 9);
    const char* val = string_value_copy(str);
    printf("%s (%zu)\n", val, strlen(val));
    assert(strlen(val) == str->len);
    for (int i = 0; i < strlen(val); i++) {
        assert(str->ptr[i] == val[i]);
    }

    string_reverse(str);
    const char* rev = string_value_copy(str);
    assert(strlen(rev) == strlen(val));

    string_t* str2 = string_create(5);
    for (char c = '1'; c <= '4'; c++) {
        string_append_char(str2, c);
    }
    string_t* str_copy = string_copy(str2);
    assert(str_copy->len == str2->len);
    assert(str_copy->ptr != str2->ptr);
    for (size_t i = 0; i < str_copy->len; i++) {
        assert(str_copy->ptr[i] == str2->ptr[i]);
    }
    string_append_char(str_copy, '5');
    assert(str_copy->len == str2->len + 1);
    assert(string_atoi(str_copy, false) == 12345);
    assert(string_atoi(str_copy, true) == 54321);

    printf("test_str passed.\n");
}

typedef struct graph_node graph_node_t;
typedef struct graph graph_t;

typedef struct graph_node {
    const char* c;
    graph_node_t** edges;
    size_t num_edges;
} graph_node_t;

typedef struct graph {
    graph_node_t* nodes;
    size_t len;
    size_t capacity;
} graph_t;

graph_t* graph_create(size_t max_nodes) {
    graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
    graph->nodes = (graph_node_t*)malloc(sizeof(graph_node_t) * max_nodes);
    graph->capacity = max_nodes;
    graph->len = 0;
    return graph;
}

graph_node_t* graph_add_node(graph_t* graph, const char* ptr) {
    graph->nodes[graph->len].c = ptr;
    graph->nodes[graph->len].edges = (graph_node_t**)malloc(sizeof(graph_node_t*) * graph->capacity);
    graph->nodes[graph->len].num_edges = 0;
    return &graph->nodes[graph->len++];
}

void graph_add_edge(graph_node_t* from, graph_node_t* to) {
    from->edges[from->num_edges++] = to;
}

void graph_init(graph_t* graph, const char* str) {
    // size_t len = strlen(str);
    // for (size_t i = 0; i < len; i++) {
    //     graph_add_node(graph, str + i);
    // }
    while (*str != '\0' && !isspace(*str)) {
        graph_add_node(graph, str++);
    }

    for (size_t i = 0; i < graph->len; i++) {
        graph_node_t* node = &graph->nodes[i];
        const char* val = str + i;
        for (size_t j = i + 1; j < graph->len; j++) {
            graph_node_t* to_node = &graph->nodes[j];
            graph_add_edge(node, to_node);
        }
    }
}

char graph_node_value(graph_node_t* node) {
    return (*node->c);
}

void graph_dfs(graph_node_t* node, size_t depth, size_t max_depth, string_t* buffer) {
    if (node == NULL) {
        return; // return what?
    }

    // printf("searching %x\n", node);
    if (node->num_edges == 0 || depth == max_depth) {
        string_append_char(buffer, graph_node_value(node));
        return;
    }

    uint64_t max_val = 0;
    string_t* max_buf = buffer;
    char node_val = graph_node_value(node);
    for (size_t i = 0; i < node->num_edges; i++) {
        string_t* buf_cpy = string_copy(buffer);
        graph_dfs(node->edges[i], depth + 1, max_depth, buf_cpy);
        string_append_char(buf_cpy, node_val);
        uint64_t int_val = string_atoi(buf_cpy, true);
        if (int_val > max_val) {
            max_val = int_val;
            max_buf = buf_cpy;
        }
        // val = node_val * 10 + val;
        // printf("%zu\n", val);
        // if (val > max_val)
        //     max_val = val;
    }
    memcpy(buffer, max_buf, sizeof(string_t));
    return;
}

// Use DFS to generate all possible n digit values from the graph.
uint64_t graph_calculate_max(graph_t* graph, int max_digits) {
    uint64_t max = 0;
    for (int i = 0; i < graph->len; i++) {
        string_t* buf = string_create(max_digits);
        graph_dfs(graph->nodes + i, 0, max_digits - 1, buf);
        uint64_t val = string_atoi(buf, true);
        if (val > max) {
            max = val;
        }
    }
    return max;
}

void test_graph() {
    // const char* str = "123456789";
    const char* str = "1234";
    int len = strlen(str);
    graph_t* graph = graph_create(len);
    graph_init(graph, str);
    assert(graph->len == len);

    for (size_t i = 0; i < graph->len; i++) {
        graph_node_t* node = &graph->nodes[i];
        const char* val = str + i;
        assert(node->c == val);
        assert(node->num_edges == len - i - 1);
    }

    uint64_t max = graph_calculate_max(graph, 2);
    assert(max == 34);
    printf("MAX VALUE: %zu\n", max);


    // for (size_t i = 0; i < len; i++) {
    //     graph_node_t* node = graph_find_node(graph, str + i);
    //     assert(node != NULL);
    //
    //     for (size_t j = j + 1; j < len; j++) {
    //         // TODO: add edges
    //         graph_node_t* to_node = graph_find_node(graph, str + j);
    //         graph_add_edge(node, to_node);
    //     }
    //     printf("(%x) %s (%zu)\n", node, node->c, node->num_edges);
    // }

    // for (size_t i = 0; i < len; i++) {
    //     graph_node_t* node = graph_find_node(graph, str + i);
    //     assert(node != NULL);
    // }
    printf("test_graph passed.\n");
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

/*
 * Part 2 is likely a tree or DAG.
 * */
int main() {
    test_str();
    test_table();
    test_bank();
    test_graph();
    
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO READ INPUT FILE.");
        exit(-1);
    }

    uint64_t total_joltage = 0;
    char buf[128];

    // PART 1
    // digit_table_t* table = digit_table_create();
    // while (fgets(buf, sizeof(buf), file) != NULL) {
    //     int len = strlen((const char*)buf);
    //     uint32_t joltage = get_max_joltage(buf, table);
    //     if (joltage > 127) {
    //         printf("ERROR: got max joltage of %d\n", joltage);
    //         digit_table_print(table);
    //         printf("%s (%i)\n", buf);
    //         assert(false);
    //     }
    //     total_joltage += joltage;
    // }

    // PART 2
    while (fgets(buf, sizeof(buf), file) != NULL) {
        printf("calculating: %s", buf);
        int len = strlen((const char*)buf);
        graph_t* graph = graph_create(len);
        graph_init(graph, buf);
        uint64_t max = graph_calculate_max(graph, 6);
        total_joltage += max;
    }

    printf("Total joltage: %zu\n", total_joltage);
}
