#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} vec2_t;

typedef struct {
    int64_t area;
    int valid;
    int ids[2];
} rect_t;

typedef struct {
    int top;
    int left;
    int bottom;
    int right;
} rectangle_t; // just to have top/left/bottom/right

typedef struct {
    vec2_t* ptr;
    size_t  cap;
    size_t  len;
} vec2_array_t;


#define N 496
static vec2_t red_tiles[N];
static int red_tile_count;

#define N_RECT ((N - 1) * N >> 1)
static rect_t rects[N_RECT];
static rectangle_t rectangles[N_RECT];
static int rect_count;
static rectangle_t boundaries[N_RECT];
static int boundaries_count;

void print_vec2(vec2_t* vec) {
    printf("%i,%i\n", vec->x, vec->y);
}

void print_rect(rect_t* rect) {
    vec2_t a = red_tiles[rect->ids[0]];
    vec2_t b = red_tiles[rect->ids[1]];
    printf("%lld, [(%i,%i), (%i,%i)]\n", rect->area,
            a.x, a.y, b.x, b.y);
}

int64_t area(const vec2_t a, const vec2_t b) {
    return ((int64_t)abs(a.x - b.x) + 1) * ((int64_t)abs(a.y - b.y) + 1);
}

rectangle_t rect_to_rectangle(const rect_t* rect) {
    vec2_t a = red_tiles[rect->ids[0]];
    vec2_t b = red_tiles[rect->ids[1]];

    rectangle_t rectangle;
    rectangle.bottom = a.y > b.y ? a.y : b.y;
    rectangle.top    = a.y > b.y ? b.y : a.y;
    rectangle.left   = a.x > b.x ? b.x : a.x;
    rectangle.right  = a.x > b.x ? a.x : b.x;
    assert(rectangle.top <= rectangle.bottom && rectangle.left <= rectangle.right);
    return rectangle;
}

int cmparea(const void* a, const void* b) {
    rect_t r1 = *(rect_t*)a;
    rect_t r2 = *(rect_t*)b;
    if (r1.area < r2.area) return 1;
    if (r1.area > r2.area) return -1;
    return 0;
}

void vec2_array_init(vec2_array_t* arr, size_t cap) {
    arr->cap = cap;
    arr->len = 0;
    arr->ptr = (vec2_t*)malloc(sizeof(vec2_t) * cap);
}

void vec2_array_free(vec2_array_t* arr) {
    arr->cap = 0;
    arr->len = 0;
    if (arr->ptr) {
        free(arr->ptr);
        arr->ptr = NULL;
    }
}

void vec2_array_push(vec2_array_t* arr, vec2_t vec) {
    if (arr->len >= arr->cap) {
        size_t new_size = arr->cap * 2;
        arr->ptr = (vec2_t*)realloc(arr->ptr, sizeof(*arr->ptr) * new_size);
        arr->cap = new_size;
    }
    arr->ptr[arr->len++] = vec;
}

void reset_state() {
    memset(red_tiles, 0, N * sizeof(*red_tiles));
    red_tile_count = 0;

    memset(rects, 0, N * sizeof(*rects));
    memset(rectangles, 0, N * sizeof(*rectangles));
    rect_count = 0;

    memset(boundaries, 0, N * sizeof(*boundaries));
    boundaries_count = 0;
}

void init_tiles(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FAILED TO READ INPUT FILE: %s\n", filename);
        exit(-1);
    }

    char buf[64];
    int x = 0, y = 0;
    vec2_t* prev = NULL;
    while (fscanf(file, "%i,%i", &x, &y) == 2) {
        vec2_t* cur = &red_tiles[red_tile_count++];
        *cur = (vec2_t) {
            .x = x,
            .y = y,
        };

        prev = cur;
    }
    vec2_t* cur = &red_tiles[0];
    fclose(file);
}

void init_boundaries() {
    int next = 0;
    for (int current = 0; current < red_tile_count; current++) {
        next = current + 1;
        if (next == red_tile_count) 
            next = 0;
        rect_t rect;
        rect.ids[0] = current;
        rect.ids[1] = next;
        rectangle_t rectangle = rect_to_rectangle(&rect);
        assert(rectangle.top == rectangle.bottom || rectangle.left == rectangle.right);
        boundaries[boundaries_count++] = rectangle;
    }
}

void init_all_rectangles() {
    for (int i = 0; i < red_tile_count - 1; i++) {
        for (int j = i + 1; j < red_tile_count; j++) {
            rect_t* rect = &rects[rect_count++];
            rect->ids[0] = i;
            rect->ids[1] = j;
            rect->area = area(red_tiles[i], red_tiles[j]);
            rect->valid = 0;
        }
    }
    qsort(rects, rect_count, sizeof(*rects), cmparea);

    for (int i = 0; i < rect_count; i++) {
        rectangles[i] = rect_to_rectangle(&rects[i]);
    }
}

// return 1 if point is not bound by the red_tiles polygon.
bool point_collision_test(const vec2_t* poly, int n, const vec2_t vec) {
    bool collision = false;
    int next = 0;
    int px = vec.x;
    int py = vec.y;
    for (int current = 0; current < n; current++) {
        next = current + 1;
        if (next == n) 
            next = 0;
        vec2_t vc = poly[current];
        vec2_t vn = poly[next];
        if (vc.x == px && vc.y == py) {
            return true;
        }

        // BEst was 6
        // if (((vc.y > py) != (vn.y > py)) && 
        //         (px < (vn.x - vc.x) * (py - vc.y) / (vn.y - vc.y) + vc.x)) {
        //     collision = !collision;
        // }
        //
        // Best was 9
        // if (((vc.y >= py && vn.y < py) || (vc.y < py && vn.y >= py)) &&
        //         (px < (vn.x-vc.x)*(py-vc.y) / (vn.y-vc.y)+vc.x)) {
        //     collision = !collision;
        // }

        if (((vc.y >= py && vn.y < py) || (vc.y < py && vn.y >= py)) &&
                (px < (vn.x - vc.x) * (py - vc.y) / (vn.y - vc.y) + vc.x)) {
            collision = !collision;
        }
    }
    return collision;
}

bool aabb_collision(const rectangle_t a, const rectangle_t b) {
    bool left =  a.right  <= b.left;
    bool right = a.left   >= b.right;
    bool above = a.bottom <= b.top;
    bool below = a.top    >= b.bottom;
    return !(left || right || above || below);
}

void test_aabb_collision() {
    rectangle_t r1 = (rectangle_t) {
        .top = 0,
        .bottom = 4,
        .left = 0,
        .right = 4,
    };

    rectangle_t r2 = (rectangle_t) {
        .top = 0,
        .bottom = 4,
        .left = 5,
        .right = 9,
    };
    assert(!aabb_collision(r1, r2));

    r2 = (rectangle_t) {
        .top = 2,
        .bottom = 6,
        .left = 2,
        .right = 6,
    };
    assert(aabb_collision(r1, r2));
}

bool rect_is_valid(const rectangle_t rect) {
    for (int k = 0; k < boundaries_count; k++) {
        if (aabb_collision(rect, boundaries[k])) {
            return false;
        }
    }
    return true;
}

int64_t part1(const char* filename) {
    reset_state();
    init_tiles(filename);
    init_all_rectangles();
    return rects[0].area;
}

int64_t part2(const char* filename) {
    reset_state();
    init_tiles(filename);
    init_boundaries();
    assert(boundaries_count == red_tile_count);
    init_all_rectangles();
    rect_t* best = NULL;
    for (int i = 0; i < rect_count; i++) {
        if (rect_is_valid(rectangles[i])) {
            best = &rects[i];
            break;
        }
    }

    if (best)
        print_rect(best);
    return best != NULL ? best->area : 0;
}

int main() {
    fprintf(stdout, "Part 1:\n");

    int64_t sample_res = part1("sample.txt");
    fprintf(stdout, "sample result: %lld\n", sample_res);
    assert(sample_res == 50);

    int64_t input_res = part1("input.txt");
    fprintf(stdout, "input result: %lld\n\n", input_res);

    test_aabb_collision();
    fprintf(stdout, "Part 2:\n");
    sample_res = part2("sample.txt");
    fprintf(stdout, "sample result: %lld\n", sample_res);
    // assert(sample_res == 24);

    // 4636582704 is too high...
    input_res = part2("input.txt");
    fprintf(stdout, "input result: %lld\n\n", input_res);
}
