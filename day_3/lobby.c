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

uint64_t graph_node_value(graph_node_t* node) {
    return (*node->c) - '0';
}

uint64_t graph_dfs(graph_node_t* node, size_t depth, size_t max_depth) {
    if (node == NULL) {
        return 0; // return what?
    }

    printf("searching %x\n", node);
    if (node->num_edges == 0 || depth == max_depth) {
        return graph_node_value(node);
    }

    uint64_t max_val = 0;
    uint64_t node_val = graph_node_value(node);
    for (size_t i = 0; i < node->num_edges; i++) {
        uint64_t val = graph_dfs(node->edges[i], depth + 1, max_depth);
        val = node_val * 10 + val;
        printf("%zu\n", val);
        if (val > max_val)
            max_val = val;
    }
    return max_val;
}

// Use DFS to generate all possible n digit values from the graph.
void graph_calculate_max(graph_t* graph, int max_digits) {

}


// This is broken for some reason.
// graph_node_t* graph_find_node(graph_t* graph, const char* val) {
//     for (size_t i; i < graph->len; i++) {
//         if (val == graph->nodes[i].c)
//             return &graph->nodes[i];
//     }
//     return NULL;
// }

void test_graph() {
    // const char* str = "123456789";
    const char* str = "1234";
    int len = strlen(str);
    graph_t* graph = graph_create(len);
    for (size_t i = 0; i < len; i++) {
        graph_add_node(graph, str + i);
        assert(graph->len == i + 1);
    }

    for (size_t i = 0; i < graph->len; i++) {
        graph_node_t* node = &graph->nodes[i];
        const char* val = str + i;
        assert(node->c == val);
        // graph_node_t* found = graph_find_node(graph, val);
        // printf("%x %x\n", node, found);
        // assert(node == found);
        for (size_t j = i + 1; j < graph->len; j++) {
            graph_node_t* to_node = &graph->nodes[j];
            graph_add_edge(node, to_node);
        }
        assert(node->num_edges == len - i - 1);
        printf("(%x) %s (%zu)\n", node, node->c, node->num_edges);
    }

    uint64_t val = graph_dfs(&graph->nodes[0], 0, 8);
    printf("MAX VALUE: %zu\n", val);


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
    test_table();
    test_bank();
    test_graph();
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
