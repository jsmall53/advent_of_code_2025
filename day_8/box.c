#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef struct {
    int x;
    int y;
    int z;
} point_t;


typedef struct {
    uint64_t distance;
    int ids[2];
} pair_t;


typedef struct {
    int* boxes;
    size_t len;
    size_t cap;
} circuit_t;

#define NUM_POINTS 1000
static int n_points = 0;
static point_t points[NUM_POINTS];

static int n_circuits = 0;
static circuit_t circuits[NUM_POINTS];
static int circuit_ids[NUM_POINTS]; // id of 0 means no circuit, so circuit_id of x must point to circuits[x - 1]

#define NUM_PAIRS ((NUM_POINTS-1) * NUM_POINTS >> 1)
static int n_pairs = 0;
static pair_t pairs[NUM_PAIRS];

void reset_state() {
    n_circuits = 0;
    memset(circuits, 0, sizeof(*circuits) * NUM_POINTS);
    memset(circuit_ids, 0, sizeof(*circuit_ids) * NUM_POINTS);

    n_pairs = 0;
    memset(pairs, 0, sizeof(*pairs) * NUM_PAIRS);

    n_points = 0;
    memset(points, 0, sizeof(*points) * NUM_POINTS);
}

void circuit_init(circuit_t* circuit, size_t cap) {
    circuit->len = 0;
    circuit->cap = cap;
    circuit->boxes = (int*)malloc(sizeof(int) * cap);
}

void circuit_reset(circuit_t* circuit) {
    circuit->len = 0;
    memset(circuit->boxes, 0, circuit->cap * sizeof(int));
}

void circuit_add_box(circuit_t* circuit, int box_id) {
    if (circuit->len >= circuit->cap) {
        size_t new_size = circuit->cap * 2;
        circuit->boxes = (int*)realloc(circuit->boxes, sizeof(int) * new_size);
        circuit->cap = new_size;
    }
    circuit->boxes[circuit->len++] = box_id;
}

void circuit_free(circuit_t* circuit) {
    if (circuit->boxes != NULL) {
        free(circuit->boxes);
        circuit->boxes = NULL;
    }
    circuit->len = 0;
    circuit->cap = 0;
}

int cmpdesc(const void* a, const void* b) {
    int* n1 = (int*)a;
    int* n2 = (int*)b;
    if (*n1 < *n2) return 1;
    else if(*n1 > *n2) return -1;
    else return 0;
}

int cmpdist(const void* a, const void* b) {
    pair_t* p1 = (pair_t*)a;
    pair_t* p2 = (pair_t*)b;
    if (p1->distance < p2->distance) return -1;
    else if (p1->distance > p2->distance) return 1;
    else return 0;
}

void point_print(point_t* point) {
    printf("%i,%i,%i\n", point->x, point->y, point->z);
}

point_t point_parse(const char* string) {
    point_t point = {0};
    int* ptr = &point.x;
    while (*string != '\0' && *string != '\n') {
        if (*string == ',') {
            ptr++;
        } else {
            *ptr = *ptr * 10 + (*string - '0');
        }
        string++;
    }
    return point;
}

size_t squared_difference(int a, int b) {
    return (a - b) * (a - b);
}

size_t euclidean_distance_squared(point_t* point1, point_t* point2) {
    return squared_difference(point1->x, point2->x) +
        squared_difference(point1->y, point2->y) +
        squared_difference(point1->z, point2->z);
}

void init_points(const char* filename) {
    FILE* file = fopen(filename, "r");
    char buf[128];
    while (fgets(buf, sizeof(buf), file)) {
        points[n_points++] = point_parse(buf);
    }
    fclose(file);
}

void init_pairs() {
    for (int i = 0; i < n_points - 1; i++) {
        for (int j = i + 1; j < n_points; j++) {
            if (i == j)
                continue;
            assert(n_pairs < NUM_PAIRS);
            pair_t* pair = &pairs[n_pairs++];
            assert(pair != NULL);
            pair->distance = euclidean_distance_squared(&points[i], &points[j]);
            pair->ids[0] = i;
            pair->ids[1] = j;
        }
    }
    qsort(pairs, n_pairs, sizeof(pair_t), cmpdist);
}

void add_box_to_circuit(int circuit, int box) {
    circuit_add_box(&circuits[circuit - 1], box);
    circuit_ids[box] = circuit;
    // printf("adding box %i into circuit %i\n", box, circuit);
}

void create_new_circuit(int box1, int box2) {
    circuit_t* circuit = &circuits[n_circuits++];
    // printf("create new circuit %i with boxes %i and %i\n", n_circuits, box1, box2);
    circuit_init(circuit, 50);
    add_box_to_circuit(n_circuits, box1);
    add_box_to_circuit(n_circuits, box2);
    // circuit_add_box(circuit, box1);
    // circuit_ids[box1] = n_circuits;
    // circuit_add_box(circuit, box2);
    // circuit_ids[box2] = n_circuits;
    assert(circuit->len == 2);
    bool found_box1 = false;
    bool found_box2 = false;
    for (int i = 0; i < circuit->len; i++) {
        int box = circuit->boxes[i];
        if (box == box1) {
            found_box1 = true;
        } else if (box == box2) {
            found_box2 = true;
        }
    }
    assert(found_box1 && found_box2);
}

void merge_two_circuits(int id1, int id2) {
    assert(id1 != id2);
    circuit_t* circuit1 = &circuits[id1 - 1];
    circuit_t* circuit2 = &circuits[id2 - 1];
    // printf("merging circuits %i(%zu) and %i(%zu)\n", id1, circuit1->len, id2, circuit2->len);
    size_t expected_len = circuit1->len + circuit2->len;

    for (int i = 0; i < circuit2->len; i++) {
        add_box_to_circuit(id1, circuit2->boxes[i]);
        // int box = circuit2->boxes[i];
        // circuit_add_box(circuit1, box);
        // circuit_ids[box] = id1;
        // printf("merging box %i into circuit %i\n", box, id1);
    }

    circuit_reset(circuit2);
    assert(circuit1->len == expected_len);
    for (int i = 0; i < n_points; i++) {
        if (circuit_ids[i] == id2) {
            printf("invalid circuit id (%i) for box %i. Should be circuit %i\n", id2, i, id1);
            assert(circuit_ids[i] != id2);
        }
    }
}

void circuits_debug_print() {
    for (int i = 0; i < n_circuits; i++) {
        circuit_t* circuit = &circuits[i];
        printf("circuit %i (%zu): ", i + 1, circuit->len);
        for (int k = 0; k < circuit->len; k++) {
            printf("%i,", circuit->boxes[k]);
        }
        printf("\n");
    }
}

void assert_circuit_accuracy() {
    for (int i = 0; i < n_points; i++) {
        int circuit_id = circuit_ids[i];
        if (circuit_id == 0)
            continue;
        circuit_t* circuit = &circuits[circuit_id - 1];
        assert(circuit != NULL);
        if (circuit->len == 0) {
            printf("circuit %i has len 0\n", circuit_id);
            assert(circuit->len > 0);
        }
        bool found = false;
        for (int k = 0; k < circuit->len; k++) {
            if (circuit->boxes[k] == i) {
                found = true;
                break;
            }
        }

        if (!found) {
            printf("\nbox %i has circuit_id %i but is not in the circuits list.\n", i, circuit_id);
            // circuits_debug_print();
            assert(false);
        }
    }
}

void connect_n_pairs(int n) {
    for (int i = 0; i < n; i++) {
        const pair_t* pair = &pairs[i];
        const int id0 = circuit_ids[pair->ids[0]];
        const int id1 = circuit_ids[pair->ids[1]];
        if (!id0 && !id1) { // neither are in a circuit, create a new one
            create_new_circuit(pair->ids[0], pair->ids[1]);
        } else if (!id0) { // id0 is not in a circuit, add to circuit of id1
            add_box_to_circuit(id1, pair->ids[0]);
        } else if (!id1) { // id1 is not in a circuit, add to circuit of id0
            add_box_to_circuit(id0, pair->ids[1]);
        } else if (id0 != id1) { // both are in different circuits, need to merge the two
            merge_two_circuits(id0, id1);
        }
    }
}

void calculate_circuit_sizes(int* circuit_size) {
    int count = 0;
    for (int i = 0; i < n_circuits; i++) {
        circuit_t* circuit = &circuits[i];
        if (circuit->len > 0) {
            circuit_size[count++] = circuit->len;
        }
    }
    assert(count >= 3);
    qsort(circuit_size, count, sizeof(*circuit_size), cmpdesc);
}

int part1(const char* filename, int connections) {
    reset_state();
    init_points(filename);
    init_pairs();
    connect_n_pairs(connections);

    int circuit_size[NUM_POINTS] = {0};
    calculate_circuit_sizes(circuit_size);
    assert_circuit_accuracy();
    return circuit_size[0] * circuit_size[1] * circuit_size[2];
}

void part2() {

}

int main() {
    int sample_result = part1("sample.txt", 10);
    printf("Part 1 sample result: %i\n", sample_result);
    assert(sample_result == 40);
    int input_result = part1("input.txt", 1000);
    // 164696 is too high...
    // 42784 is wrong too...
    printf("Part 1 input result: %i\n", input_result);
    part2();
}
