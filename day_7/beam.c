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

typedef struct node node_t;
typedef struct node {
    char id;
    uint16_t pos;
    node_t* adj[2];
} node_t;

size_t make_node_id(int line_num, int pos) {
    return line_num * 1000000000 + pos;
}

node_t* node_init(node_t* node, uint16_t pos, char id) {
    node->id = id;
    node->pos = pos;
    node->adj[0] = NULL;
    node->adj[1] = NULL;
    return node;
}

void node_add_edge(node_t* from, node_t* to) {
    if (from->adj[0] == NULL) {
        from->adj[0] = to;
    } else if (from->adj[1] == NULL) {
        from->adj[1] = to;
    } else {
        printf("Node has more edges than expected.\n");
        assert(false);
    }
}

node_t* node_edge(node_t* node, uint16_t edge) {
    assert(edge < 2);
    return node->adj[edge];
}

typedef struct {
    node_t* nodes;
    size_t  cap;
    size_t  len;
    size_t  stride;
} node_array_t;

node_array_t* node_array_create(size_t cap, size_t stride) {
    node_array_t* arr = (node_array_t*)malloc(sizeof(node_array_t));
    arr->nodes = (node_t*)malloc(sizeof(node_t) * cap);
    arr->cap = cap;
    arr->len = 0;
    arr->stride = stride;
    return arr;
}

void node_array_free(node_array_t* arr) {
    free(arr->nodes);
    arr->nodes = NULL;
    arr->cap = 0;
    arr->len = 0;
    free(arr);
}

node_t* node_array_push(node_array_t* arr, char id) {
    if (arr->len >= arr->cap) {
        size_t new_cap = arr->cap * 2;
        node_t* old_nodes = arr->nodes;
        node_t* new_nodes = (node_t*)malloc(sizeof(node_t) * new_cap);
        memcpy(new_nodes, old_nodes, sizeof(node_t) * arr->cap);
        free(old_nodes);
        arr->nodes = new_nodes;
        arr->cap = new_cap;
    }

    node_t* node = arr->nodes + arr->len++;
    node_init(node, arr->len - 1, id);
    return node;
}

void node_array_debug(node_array_t* node_array) {
    printf("Len: %zu\n", node_array->len);
    printf("Stride: %zu\n", node_array->stride);

    int row = 0;
    int col = 0;
    int num_leaves = 0;
    node_t* start = NULL;
    for (int i = 0; i < node_array->len; i++) {
        node_t* node = node_array->nodes + i;
        printf("%c", node->id);
        if(node->id == CHAR_INIT_BEAM) {
            start = node;
        }

        if (node_edge(node, 0) == NULL && node_edge(node, 1) == NULL) {
            num_leaves++;
        }
        if ((i + 1) % node_array->stride == 0) {
            row++;
            col = 0;
            printf("\n");
        } else {
            col++;
        }
    }
    printf("Leaves: %i\n", num_leaves);
    assert(start != NULL);
    node_t* node = start;
    while (node != NULL) {
        node = node_edge(node, 0);
    }
    assert(true);
}

size_t descend_all_paths(node_t* node, size_t* memo) {
    if (memo[node->pos] > 0) {
        return memo[node->pos];
    }

    if (node_edge(node, 0) == NULL && node_edge(node, 1) == NULL) {
        memo[node->pos] = 1;
        return 1;
    }

    size_t paths = 0;
    for (uint16_t i = 0; i < 2; i++) {
        node_t* edge = node_edge(node, i);
        if (edge) 
            paths += descend_all_paths(edge, memo);
    }
    memo[node->pos] = paths;
    return paths;
}

void print_all_nodes(node_t* node) {
    if (node == NULL)
        return;
    printf("%c\n", node->id);
    for (int i = 0; i < 2; i++) {
        print_all_nodes(node_edge(node, i));
    }
    return;
}

size_t calculate_timelines(node_array_t* node_array) {
    size_t count = 0;
    node_t* start = NULL;
    for (size_t i = 0; i < node_array->len; i++) {
        node_t* node = node_array->nodes + i;
        if (node->id == CHAR_INIT_BEAM) {
            start = node;
            break;
        }
    }
    assert(start != NULL);
    size_t* memo = (size_t*)malloc(sizeof(size_t) * node_array->len);
    memset(memo, 0, node_array->len * sizeof(size_t));
    return descend_all_paths(start, memo);
}

node_array_t* part2_read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FAILED TO READ INPUT FILE\n");
        exit(1);
    }

    char buf[BUFFER_SIZE] = {0};
    node_array_t* node_array = NULL;
    size_t line_num = 0;
    size_t stride = 0;
    while (fgets(buf, sizeof(buf), file) != NULL) {
        if (node_array == NULL) {
            size_t buf_len = strlen(buf);
            node_array = node_array_create(buf_len * 4, buf_len - 1);
        }
        line_num++;
        for (size_t i = 0; i < strlen(buf); i++) {
            char c = buf[i];
            if (isspace(c))
                continue;

            node_t* node = node_array_push(node_array, c);
        }
    }
    fclose(file);
    return node_array;
}

void init_edges(node_array_t* node_array) {
    int row = 0;
    int col = 0;
    size_t stride = node_array->stride;
    for (size_t i = 0; i < node_array->len; i++) {
        node_t* node = node_array->nodes + i;

        assert(row * stride + col == i);
        switch (node->id) {
            case '.':
            case CHAR_INIT_BEAM:
            {
                size_t pos_below = (row + 1) * stride + col;
                if (pos_below < node_array->len) {
                    node_t* below = node_array->nodes + pos_below;
                    node_add_edge(node, below);
                }
                break;
            }
            case CHAR_SPLIT:
            {
                if (col > 0) {
                    size_t pos_left_below = (row + 1) * stride + (col - 1);
                    if (pos_left_below < node_array->len) {
                        node_t* target = node_array->nodes + pos_left_below;
                        node_add_edge(node, target);
                    }
                }

                if (col < stride - 1) {
                    size_t pos_right_below = (row + 1) * stride + (col + 1);
                    if (pos_right_below < node_array->len) {
                        node_t* target = node_array->nodes + pos_right_below;
                        node_add_edge(node, target);
                    }
                }

                break;
            }
            default:
                printf("Unhandled token: '%c'\n", node->id);
                assert(false);
                break;
        }

        if ((i + 1) % stride == 0) {
            row++;
            col = 0;
        } else {
            col++;
        }
    }
}



void part2() {
    node_array_t* sample_array = part2_read_file("sample.txt");
    init_edges(sample_array);
    assert(calculate_timelines(sample_array) == 40);
    printf("part2 sample passed.\n");

    node_array_t* node_array = part2_read_file("input.txt");
    init_edges(node_array);
    printf("part2 result: %zu\n", calculate_timelines(node_array));
}

int main() {
    part1();
    part2();
}
