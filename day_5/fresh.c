#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    size_t start;
    size_t end;
} range_t;

bool range_contains(range_t* range, size_t n) {
    if (n >= range->start && n <= range->end)
        return true;
    return false;
}

size_t range_count(range_t* range) {
    if (range->start == 0 && range->end == 0)
        return 0;
    return range->end - range->start + 1;
}

bool range_overlap(range_t* range1, range_t* range2) {
    size_t a1 = range1->start;
    size_t a2 = range1->end;
    size_t b1 = range2->start;
    size_t b2 = range2->end;

    return (a1 <= b2) && (b1 <= a2);
}

// bool range_overlap_old(range_t* range1, range_t* range2) {
//     range_t* smaller, *larger;
//     if (range1->start < range2->start) {
//         smaller = range1;
//         larger = range2;
//     } else {
//         smaller = range2;
//         larger = range1;
//     }
//
//     // equal range starts
//     // 1-3
//     // 1-4
//     if (smaller->start == larger->start || smaller->end == larger->end) {
//         return true;
//     }
//
//     // no overlap
//     if (smaller->end < larger->start && smaller->end < range2->end) {
//         return false;
//     }
//
//     // 34-50
//     // 35-100
//     if (smaller->end > larger->start && smaller->end < larger->end) {
//         return true;
//     }
//
//     // Engulfing ranges, one fits inside the other
//     // 34-50
//     // 35-49
//     if (smaller->end >= larger->start && smaller->end > larger->end) { 
//         return true;
//     }
//
//     return false;
// }

range_t range_consolidate(range_t* range1, range_t* range2) {
    return (range_t) {
        .start = range1->start < range2->start ? range1->start : range2->start,
        .end   = range1->end > range2->end ? range1->end : range2->end,
    };
}

// range_t range_consolidate_old(range_t* range1, range_t* range2) {
//     range_t* smaller, *larger;
//     if (range1->start < range2->start) {
//         smaller = range1;
//         larger = range2;
//     } else {
//         smaller = range2;
//         larger = range1;
//     }
//
//     // equal range starts
//     if (smaller->start == larger->start) {
//         range_t range;
//         range.start = smaller->start;
//         range.end = smaller->end >= larger->end ? smaller->end : larger->end;
//         return range;
//     }
//
//     // equal range ends 
//     if (smaller->end == larger->end) {
//         range_t range;
//         range.start = smaller->start;
//         range.end = smaller->end;
//         return range;
//     }
//
//     // no overlap
//     if (smaller->end < larger->start && smaller->end < range2->end) {
//         return (range_t){0};
//     }
//
//     // 34-50
//     // 35-100
//     if (smaller->end > larger->start && smaller->end < larger->end) {
//         return (range_t){
//             .start = smaller->start,
//             .end   = larger->end,
//         };
//     }
//
//     // Engulfing ranges, one fits inside the other
//     // 34-50
//     // 35-49
//     if (smaller->end >= larger->start && smaller->end >= larger->end) { 
//         return (range_t){
//             .start = smaller->start,
//             .end   = smaller->end,
//         };
//     }
//
//     return (range_t){0};
// }

void test_range() {
    range_t range1 = { .start = 1, .end = 7 };
    assert(range_count(&range1) == 7);
    range_t range2 = { .start = 6, .end = 10};
    assert(range_count(&range2) == 5);
    // 1-7
    // 6-10
    assert(range_overlap(&range1, &range2));
    assert(range_overlap(&range2, &range1));
    range_t range3 = range_consolidate(&range1, &range2);
    assert(range3.start == range1.start && range3.end == range2.end);

    range1.start = 1;
    range1.end   = 3;
    assert(range_count(&range1) == 3);
    // 1-3
    // 6-10
    assert(!range_overlap(&range1, &range2));
    assert(!range_overlap(&range2, &range1));

    range1.start = 7;
    range1.end   = 9;
    assert(range_count(&range1) == 3);
    // 7-9
    // 6-10
    assert(range_overlap(&range1, &range2));
    assert(range_overlap(&range2, &range1));
    range3 = range_consolidate(&range1, &range2);
    assert(range3.start == range2.start && range3.end == range2.end);

    // 121173177681726-127288474336774
    range1.start = 121173177681726;
    range1.end = 127288474336774;
    range2.start = 121173177681726;
    range2.end = 121173177681726;
    assert(range_overlap(&range1, &range2));
    range_t eq = range_consolidate(&range1, &range2);
    assert(eq.start == range1.start && eq.end == range1.end);


    // 447008091749  - 1670555330556
    // 1151221132517 - 1356033568084
    range1.start = 447008091749;
    range1.end   = 1670555330556;
    range2.start = 1151221132517;
    range2.end   = 1356033568084;
    assert(range_overlap(&range2, &range1));
    range3 = range_consolidate(&range2, &range1);
    assert(range3.start == range1.start && range3.end == range1.end);

    // 706001751286  - 1356033568084
    // 1151221132517 - 1356033568084
    range1.start = 706001751286;
    range1.end   = 1356033568084;
    range2.start = 1151221132517;
    range2.end   = 1356033568084;
    assert(range_overlap(&range1, &range2));

    // 2360603309607-2810323474576
    // 2810323474576-3723043099728
    range1.start = 2360603309607;
    range1.end   = 2810323474576;
    range2.start = 2810323474576;
    range2.end   = 3723043099728;
    assert(range_overlap(&range1, &range2));

    printf("test_range passed.\n");
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

typedef struct range_node range_node_t;
typedef struct range_node {
    range_t range;
    range_node_t* left;
    range_node_t* right;
} range_node_t;

typedef struct {
    range_node_t* ranges;
    size_t len;
    size_t cap;
    range_node_t* root;
} bst_t;

void bst_debug_print(range_node_t* node);

bst_t* bst_create(size_t cap) {
    bst_t* bst = (bst_t*)malloc(sizeof(bst_t));
    bst->len = 0;
    bst->root = NULL;
    bst->ranges = (range_node_t*)malloc(sizeof(range_node_t) * cap);
    bst->cap = cap;
    return bst;
}

void bst_free(bst_t* bst) {
    free(bst->ranges);
    bst->ranges = NULL;
    bst->root = NULL;
    bst->cap = 0;
    bst->len = 0;
    free(bst);
}

range_node_t* bst_node_push(bst_t* bst, range_t range) {
    if (bst->len >= bst->cap) {
        size_t new_cap = bst->cap * 2;
        bst->ranges = (range_node_t*)realloc(bst->ranges, new_cap * sizeof(range_node_t));
        bst->cap = new_cap;
    }

    range_node_t* node = &bst->ranges[bst->len++];
    node->range = range;
    node->left = NULL;
    node->right = NULL;
    return node;
}

range_node_t* bst_node_insert_recursive(range_node_t* node, range_node_t* new) {
    bool overlap = range_overlap(&node->range, &new->range);
    if (!overlap && new->range.start < node->range.start) {
        if (node->left != NULL) {
            return bst_node_insert_recursive(node->left, new);
        } else { // node == left
            node->left = new;
            return node->left;
        }
    }

    if (!overlap && new->range.start > node->range.start) {
        if (node->right != NULL) {
            return bst_node_insert_recursive(node->right, new);
        } else {
            node->right = new;
            return node->right;
        }
    }

    if (overlap) {
        range_t new_range = range_consolidate(&node->range, &new->range);
        node->range = new_range;
        return node;
    }
    printf("Something weird:\nnode: %zu-%zu\nnew: %zu-%zu\n", 
            node->range.start, node->range.end, new->range.start, new->range.end);
    assert(false);
    return NULL;
}

void bst_node_prune(range_node_t* node) {
    if (node == NULL)
        return;

    bst_node_prune(node->left);
    bst_node_prune(node->right); // is this correct??
                                 //
    bool debug = true;
    // if (node->range.start == 1151221132517) {
    // if (node->range.start == 447008091749) {
    //     printf("FOUND DEBUG CASE\n");
    //     bst_debug_print(node);
    //     printf("\n");
    //     debug = true;
    // }

    if (node->left != NULL && range_overlap(&node->range, &node->left->range)) {
        range_t new_range = range_consolidate(&node->range, &node->left->range);
        node->range = new_range;
        range_node_t* tmp_right = node->left->right;
        range_node_t* tmp_left = node->left->left;
        if (tmp_right != NULL) {
            node->left = tmp_right;
            tmp_right->left = tmp_left;
        } else {
            node->left = tmp_left;
        }
    }

    if (node->right != NULL && range_overlap(&node->range, &node->right->range)) {
        range_t new_range = range_consolidate(&node->range, &node->right->range);
        node->range = new_range;
        range_node_t* tmp_right = node->right->right;
        range_node_t* tmp_left  = node->right->left;
        if (tmp_right != NULL) {
            node->right = tmp_right;
            tmp_right->left = tmp_left;
        } else {
            node->right = tmp_left;
        }
    }
}

void bst_check_tree(range_node_t* node) {
    if (node == NULL)
        return;

    bst_check_tree(node->left);
    if (node->left != NULL) {
        assert(node->range.start > node->left->range.end);
        // assert(node->range.start > node->left->range.start);
    }

    if (node->right != NULL) {
        assert(node->range.end < node->right->range.start);
        // if (node->range.start >= node->right->range.start) {
        //     printf("CHECK TREE FAILED:\nnode: %zu-%zu\nright: %zu-%zu\n", 
        //             node->range.start, node->range.end, node->right->range.start, node->right->range.end);
        //     assert(false);
        // }
    }
    bst_check_tree(node->right);
}

range_node_t* bst_node_insert(bst_t* bst, range_t range) {
    if (bst->root == NULL) {
        bst->root = bst_node_push(bst, range);
        return bst->root;
    }

    range_node_t* node = bst_node_push(bst, range);
    bst_node_insert_recursive(bst->root, node);
    bst_node_prune(bst->root);
    return node;
}

void bst_debug_print(range_node_t* node) {
    if (node == NULL)
        return;

    bst_debug_print(node->left);
    printf("%zu-%zu\n", node->range.start, node->range.end);
    bst_debug_print(node->right);
}

size_t bst_count_ranges(range_node_t* node) {
    if (node == NULL)
        return 0;
    size_t total = 0;
    total += bst_count_ranges(node->left);
    total += range_count(&node->range);
    total += bst_count_ranges(node->right);
    return total;
}

void test_bst() {
    bst_t* bst = bst_create(12);
    bst_node_insert(bst, (range_t){ .start = 3, .end = 5 } );
    assert(bst->root != NULL);
    bst_check_tree(bst->root);
    assert(bst->root->range.start == 3 && bst->root->range.end == 5);
    bst_node_insert(bst, (range_t){ .start = 10, .end = 14 } );
    assert(bst->root->right->range.start == 10 && bst->root->right->range.end == 14);
    bst_check_tree(bst->root);
    bst_node_insert(bst, (range_t){ .start = 16, .end = 20 } );
    assert(bst->root->right->right->range.start == 16 && bst->root->right->right->range.end == 20);
    bst_check_tree(bst->root);
    bst_node_insert(bst, (range_t){ .start = 12, .end = 18 } );
    bst_check_tree(bst->root);
    assert(bst->root->right->range.start == 10 && bst->root->right->range.end == 20);
    assert(bst_count_ranges(bst->root) == 14);


    bst_node_insert(bst, (range_t){
            .start = 1151221132517,
            .end   = 1356033568084 });
    bst_node_insert(bst, (range_t){ 
            .start = 447008091749,
            .end = 1670555330556 });
    bst_check_tree(bst->root);
    bst_node_insert(bst, (range_t){
            .start = 12345,
            .end   = 15000 });
    bst_node_insert(bst, (range_t){ 
            .start = 12345,
            .end   = 15001 });
    bst_check_tree(bst->root);
    printf("test_bst passed.\n");
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
    test_range();
    test_bst();

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
    fclose(file);

    file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("FAILED TO OPEN INPUT FILE.\n");
        exit(-1);
    }
    range_array_t* arr = range_array_create(150);
    while(fgets(buf, sizeof(buf), file) != NULL) {
        line += 1;
        if (isspace(*buf)) {
            break;
        } else { // parse id range
            int split_idx = find_char(buf, '-');
            assert(split_idx != -1);
            size_t start = string_to_id(buf, 0, split_idx);
            size_t end = string_to_id(buf, split_idx + 1, strlen(buf) - 1);
            range_array_add_range(arr, start, end);
        }
    }
    assert(arr->len > 0);

    size_t num_fresh_ids = 0;
    bst_t* bst = bst_create(arr->len);
    for (size_t i = 0; i < arr->len; i++) {
        bst_node_insert(bst, arr->ptr[i]);
        bst_check_tree(bst->root);
    }
    bst_node_prune(bst->root);
    // printf("\nFINAL TREE:\n\n");
    // bst_debug_print(bst->root);

    // 118828288127539 is too low 118_828_288_127_539
    // 118828288127590 is too low
    // 344300757540305 is not right, feels close...
    // 333994202581208 is not right, feels closer...
    num_fresh_ids = bst_count_ranges(bst->root);
    printf("Part 2: %zu\n", num_fresh_ids);
}
