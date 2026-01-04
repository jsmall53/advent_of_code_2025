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

size_t range_count(range_t* range) {
    assert(range->end - range->start >= 0);
    return range->end - range->start + 1;
}

bool range_overlap(range_t* range1, range_t* range2) {
    size_t a1 = range1->start;
    size_t a2 = range1->end;
    size_t b1 = range2->start;
    size_t b2 = range2->end;

    return (a1 <= b2) && (b1 <= a2);
}

range_t range_consolidate(range_t* range1, range_t* range2) {
    return (range_t) {
        .start = range1->start < range2->start ? range1->start : range2->start,
        .end   = range1->end > range2->end ? range1->end : range2->end,
    };
}

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

    // 60627233829569-67301045866627
    // 67014797628214-70234710680711
    range1.start = 60627233829569;
    range1.end   = 67301045866627;
    range2.start = 67014797628214;
    range2.end   = 70234710680711;
    assert(range_overlap(&range1, &range2));
    range_t other = range_consolidate(&range1, &range2);
    assert(other.start == range1.start && other.end == range2.end);

    assert(range_count(&(range_t){.start = 1, .end = 1}) == 1);

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
        node->range = range_consolidate(&node->range, &new->range);
        return node;
    }
    printf("Something weird:\nnode: %zu-%zu\nnew: %zu-%zu\n", 
            node->range.start, node->range.end, new->range.start, new->range.end);
    assert(false);
    return NULL;
}

range_node_t* bst_node_delete(range_node_t* node) {
    if (node == NULL)
        return NULL;

    if (node->left == NULL) {
        return node->right;
    }

    if (node->right == NULL) {
        return node->left;
    }

    range_node_t* successor = node->right;
    while (successor->left) {
        successor = successor->left;
    }
    node->range = successor->range;
    bst_node_delete(successor);
    return node;
}

void bst_node_prune(range_node_t* node) {
    if (node == NULL)
        return;

    if (node->left != NULL && range_overlap(&node->range, &node->left->range)) {
        node->range = range_consolidate(&node->range, &node->left->range);
        range_node_t* old_left = node->left;
        if (old_left->left && !old_left->right) {
            node->left = old_left->left;
        } else if(!old_left->left && old_left->right) {
            node->left = old_left->right;
        } else if (!old_left->left && !old_left->right) {
            node->left = NULL;
        } else {
            assert(false);
        }
        // range_node_t* tmp_right = node->left->right;
        // range_node_t* tmp_left = node->left->left;
        // if (tmp_right != NULL) {
        //     node->left = tmp_right;
        //     tmp_right->left = tmp_left;
        // } else {
        //     node->left = tmp_left;
        // }
        // if (old_left->right != NULL) {
        //     node->left = old_left->right;
        //     node->left->left = old_left->left;
        // } else {
        //     node->left = old_left->left;
        // }
    }

    if (node->right != NULL && range_overlap(&node->range, &node->right->range)) {
        node->range = range_consolidate(&node->range, &node->right->range);
        range_node_t* old_right = node->right;
        if (old_right->left && !old_right->right) 
            node->right = old_right->left;
        else if (!old_right->left && old_right->right)
            node->right = old_right->right;
        else if (!old_right->left && !old_right->right) {
            node->right = NULL;
        } else {
            assert(false);
        }
        // range_node_t* tmp_right = node->right->right;
        // range_node_t* tmp_left  = node->right->left;
        // if (tmp_left != NULL) {
        //     node->right = tmp_left;
        //     tmp_left->right = tmp_right;
        // } else {
        //     node->right = tmp_left;
        // }
        // if (old_right->left != NULL) {
        //     node->right = old_right->left;
        //     node->right->right = old_right->right;
        // } else {
        //     node->right = old_right->left;
        // }
    }

    bst_node_prune(node->left);
    bst_node_prune(node->right);
}

bool bst_check_tree(range_node_t* node) {
    if (node == NULL)
        return true;

    bool ret = true;
    ret = bst_check_tree(node->left);
    ret = bst_check_tree(node->right);
    if (node->range.start > node->range.end)
        ret = false;

    if (node->left != NULL && node->range.start <= node->left->range.end) {
        // assert(node->range.start > node->left->range.end);
        ret = false;
    }

    if (node->right != NULL && node->range.end >= node->right->range.start) {
        // assert(node->range.end < node->right->range.start);
        ret = false;
    }
    return ret;
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

size_t bst_count_ids(range_node_t* node) {
    if (node == NULL)
        return 0;
    size_t left_count = bst_count_ids(node->left);
    size_t right_count = bst_count_ids(node->right);
    size_t node_count = range_count(&node->range);
    return left_count + node_count + right_count;
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
    assert(bst_count_ids(bst->root) == 14);


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

    // 60627233829569-67301045866627
    // 67014797628214-70234710680711
    // These aren't merging in the input2.txt case
    bst_node_insert(bst, (range_t){ 
            .start = 60627233829569,
            .end   = 67301045866627 });
    bst_node_insert(bst, (range_t){ 
            .start = 67014797628214,
            .end   = 70234710680711 });
    bst_check_tree(bst->root);
    bst_debug_print(bst->root);
    printf("test_bst passed.\n");
    assert(false);
}

typedef struct list_node list_node_t;
typedef struct list_node {
    range_t range;
    list_node_t* next;
} list_node_t;

typedef struct {
    list_node_t* ranges;
    size_t len;
    size_t cap;
    list_node_t* head;
} range_list_t;

range_list_t* range_list_create(size_t cap) {
    range_list_t* list = (range_list_t*)malloc(sizeof(range_list_t));
    list->len = 0;
    list->head = NULL;
    list->ranges = (list_node_t*)malloc(sizeof(list_node_t) * cap);
    list->cap = cap;
    return list;
}

void range_list_free(range_list_t* list) {
    free(list->ranges);
    list->ranges = NULL;
    list->head = NULL;
    list->cap = 0;
    list->len = 0;
    free(list);
}

list_node_t* range_list_node_create(range_list_t* list, range_t range) {
    if (list->len >= list->cap) {
        size_t new_cap = list->cap * 2;
        list->ranges = (list_node_t*)realloc(list->ranges, sizeof(list_node_t) * new_cap);
        list->cap = new_cap;
    }

    list_node_t* node = &list->ranges[list->len++];
    node->range = range;
    node->next  = NULL;
    return node;
}

list_node_t* range_list_push_sorted(range_list_t* list, range_t range) {
    if (list->head == NULL) {
        list->head = range_list_node_create(list, range);
        return list->head;
    }
    // 3 -> 4 -> 7;
    // insert 5.
    // 1) node = 3, prev = NULL
    // 2) node = 4, prev = 3
    // 3) node = 7, prev = 4 // break here and insert between prev and node.
    // check for merges.
    list_node_t* node = list->head;
    list_node_t* prev = NULL;

    while (node != NULL) {
        if (range.start < node->range.start) {
            break;
        }
        prev = node;
        node = node->next;
    }

    list_node_t* new_node = range_list_node_create(list, range);
    if (prev == NULL) {
        new_node->next = node;
        list->head = new_node;
    } else {
        new_node->next = prev->next;
        prev->next = new_node;
    }
    return new_node;
}

int range_list_merge(range_list_t* list) {
    list_node_t* node = list->head;
    list_node_t* next = node->next;
    int merged = 0;
    while (next != NULL) {
        if (range_overlap(&node->range, &next->range)) {
            merged++;
            node->range = range_consolidate(&node->range, &next->range);
            node->next  = node->next->next;
            next = node->next;
            continue; // if we merge this iter, don't increment node
        }
        node = node->next;
        next = node->next;
    }
    return merged;
}

void range_list_debug_print(range_list_t* list) {
    list_node_t* node = list->head;
    while(node != NULL) {
        printf("%zu-%zu\n", node->range.start, node->range.end);
        node = node->next;
    }
    printf("\n");
}

size_t range_list_count_ids(range_list_t* list) {
    size_t accum = 0;
    list_node_t* node = list->head;
    while (node != NULL) {
        accum += range_count(&node->range);
        node = node->next;
    }
    return accum;
}

void test_range_list() {
    // THIS ISN'T WORKING, INSERTION IS BROKEN
    range_list_t* list = range_list_create(8);
    range_list_push_sorted(list, (range_t){.start = 3, .end = 5});
    range_list_push_sorted(list, (range_t){.start = 10, .end = 14});
    range_list_push_sorted(list, (range_t){.start = 16, .end = 20});
    range_list_push_sorted(list, (range_t){.start = 12, .end = 18});

    range_list_push_sorted(list, (range_t){.start = 111, .end = 112});
    range_list_push_sorted(list, (range_t){.start = 113, .end = 114});
    range_list_push_sorted(list, (range_t){.start = 109, .end = 110});

    assert(range_list_merge(list) == 2);
    range_list_debug_print(list);
    printf("test_range_list passed.\n");
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
    // test_bst();
    test_range_list();
    // exit(1);
    const char* input_name = "input.txt";
    FILE* file = fopen(input_name, "r");
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

    file = fopen(input_name, "r");
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
    range_list_t* list = range_list_create(arr->len);
    for (size_t i = 0; i < arr->len; i++) {
        range_list_push_sorted(list, arr->ptr[i]);
        // range_list_debug_print(list);
    }
    range_list_merge(list);
    printf("\nFINAL LIST (%zu):\n\n", list->len);
    range_list_debug_print(list);
    num_fresh_ids = range_list_count_ids(list);

    // 118828288127539 is too low 118_828_288_127_539
    // 118828288127590 is too low
    // 344300757540305 is not right, feels close...
    // 333994202581208 is not right, feels closer...
    // num_fresh_ids = bst_count_ids(list->root);
    // for input2.txt, answer is 344486348901788
    // (i am getting 346228877525236) need to find the bug...

    printf("Part 2: %zu\n", num_fresh_ids);
    fclose(file);
}
