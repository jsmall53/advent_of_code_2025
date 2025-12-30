#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef struct grid_node grid_node_t;
typedef struct grid_node {
    int val;
    size_t n; // num adjacent nodes, will be 8 for most in the grid.
    size_t cap;
    grid_node_t** adj;
} grid_node_t;

#define DEFAULT_NODE_CAP 8
#define DEFAULT_NODE_VAL -1

#define NODE_EMPTY      0
#define NODE_PAPER      (1 << 1) 
#define NODE_ACCESSIBLE (1 << 2)

void grid_node_destroy(grid_node_t* node) {
    node->val = DEFAULT_NODE_VAL;
    node->n = 0;
    free(node->adj);
    node->adj = NULL;
    node->cap = 0;
}

typedef struct {
    size_t n;
    size_t cap;
    size_t stride;
    grid_node_t* nodes;
} grid_t;

grid_t* grid_create(size_t stride, size_t initial_cap) {
    grid_t* grid = (grid_t*)malloc(sizeof(grid_t));
    grid->stride = stride;
    grid->n = 0;
    grid->nodes = (grid_node_t*)malloc(sizeof(grid_node_t) * initial_cap);
    grid->cap = initial_cap;
    return grid;
}

void grid_destroy(grid_t* grid) {
    free(grid->nodes);
    grid->nodes = NULL;
    grid->cap = 0;
    grid->n = 0;
}

size_t grid_len(grid_t* grid) {
    return grid->n;
}

grid_node_t* grid_add_node(grid_t* grid, int val) {
    if (grid->n >= grid->cap) {
        size_t new_cap = grid->cap * 2;
        grid->nodes = (grid_node_t*)realloc(grid->nodes, new_cap * sizeof(grid_node_t));
        grid->cap = new_cap;
    }

    grid_node_t* node = &grid->nodes[grid->n++];
    node->val = val;
    node->n = 0;
    node->adj = (grid_node_t**)malloc(sizeof(grid_node_t*) * DEFAULT_NODE_CAP);
    node->cap = DEFAULT_NODE_CAP;
    return node;
}

void grid_node_add_adj(grid_node_t* node, grid_node_t* adj) {
    if (node->n >= node->cap) {
        size_t new_cap = node->cap * 2;
        node->adj = (grid_node_t**)realloc(node->adj, new_cap * sizeof(grid_node_t*));
        node->cap = new_cap;
    }
    node->adj[node->n++] = adj;
}


void grid_node_set_flags(grid_node_t* node, int flags) {
    node->val |= flags;
}

void grid_node_unset_flags(grid_node_t* node, int flags) {
    node->val &= !flags;
}

void grid_node_set_accessible(grid_node_t* node) {
    grid_node_set_flags(node, NODE_ACCESSIBLE);
}

bool grid_node_has_flags(grid_node_t* node, int flags) {
    return (node->val & flags) == flags;
}

bool grid_node_is_accessible(grid_node_t* node) {
    return grid_node_has_flags(node, NODE_ACCESSIBLE);
}

bool grid_node_has_paper(grid_node_t* node) {
    return grid_node_has_flags(node, NODE_PAPER);
}

void test_grid() {
    size_t len = 9;
    grid_t* grid = grid_create(3, len);
    for (size_t i = 0; i < len; i++) {
        grid_add_node(grid, NODE_PAPER);
    }
    assert(grid->n == len);
    assert(grid_node_has_flags(grid->nodes, NODE_PAPER));
    grid_node_set_accessible(grid->nodes);
    assert(grid_node_has_flags(grid->nodes, NODE_ACCESSIBLE));
    assert(grid_node_has_flags(grid->nodes, NODE_ACCESSIBLE | NODE_PAPER));
    assert(!grid_node_has_flags(grid->nodes + 1, NODE_ACCESSIBLE));
    assert(!grid_node_has_flags(grid->nodes + 1, NODE_ACCESSIBLE | NODE_PAPER));
    grid_node_unset_flags(grid->nodes, NODE_ACCESSIBLE);
    assert(!grid_node_has_flags(grid->nodes, NODE_ACCESSIBLE));

    printf("test_grid passed.\n");
}

void execute_part1(grid_t* grid) {
    for (size_t idx = 0; idx < grid->n; idx++) {
        int row = idx / grid->stride;
        int col = idx % grid->stride;

        grid_node_t* node = &grid->nodes[idx];
        int adj_has_paper = 0;
        for (int i = row - 1; i <= row + 1; i++) {
            for (int j = col - 1; j <= col + 1; j++) {
                if (i < 0 || j < 0) continue;

                size_t index = (i * grid->stride) + j;
                if (index > grid->n - 1 || index == idx) continue;
                
                grid_node_t* adj = &grid->nodes[index];
                if (grid_node_has_paper(adj)) {
                    adj_has_paper += 1;
                }
            }
        }

        if (adj_has_paper < 4) {
            grid_node_set_accessible(node);
        }
    }

    int num_accessible = 0;
    for (size_t i = 0; i < grid->n; i++) {
        if (grid_node_has_flags(grid->nodes + i, NODE_PAPER | NODE_ACCESSIBLE))
            num_accessible += 1;
    }
    printf("Part 1 answer: %i\n", num_accessible);
}

void execute_part2(grid_t* grid) {
    int total_changed = 0;
    int num_changed = 0xFF; // some default?
                            //
    while (num_changed > 0) {
        num_changed = 0;
        for (size_t node_idx = 0; node_idx < grid_len(grid); node_idx++) {
            int row = node_idx / grid->stride;
            int col = node_idx % grid->stride;

            grid_node_t* node = &grid->nodes[node_idx];
            int adj_has_paper = 0;

            for (int i = row - 1; i <= row + 1; i++) {
                for (int j = col - 1; j <= col + 1; j++) {
                    if (i < 0 || j < 0) continue;

                    size_t index = (i * grid->stride) + j;
                    if (index > grid->n - 1 || index == node_idx) continue;

                    grid_node_t* adj = &grid->nodes[index];
                    if (grid_node_has_paper(adj)) {
                        adj_has_paper += 1;
                    }
                }
            }

            if (adj_has_paper < 4) {
                grid_node_set_accessible(node);
            }
        }

        for (size_t i = 0; i < grid->n; i++) {
            if (grid_node_has_flags(grid->nodes + i, NODE_PAPER | NODE_ACCESSIBLE)) {
                grid_node_unset_flags(grid->nodes + i, NODE_PAPER);
                num_changed++;
            }
        }
        total_changed += num_changed;
        printf("%i (%i).\n", num_changed, total_changed);
    }
}

int main() {
    test_grid();
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO OPEN INPUT FILE.\n");
        exit(-1);
    }

    char buf[256];
    grid_t* grid = NULL;
    while(fgets(buf, sizeof(buf), file) != NULL) {
        if (grid == NULL) {
            size_t stride = strlen(buf);
            grid = grid_create(stride, 1024);
        }

        for (size_t i = 0; i < strlen(buf); i++) {
            int val = (*(buf + i) == '@') ? NODE_PAPER : NODE_EMPTY;
            grid_add_node(grid, val);
        }
    }

    execute_part1(grid);
    execute_part2(grid);
    grid_destroy(grid);
}
