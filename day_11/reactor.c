#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


#define BOOL int
#define FALSE 0
#define TRUE  1

#define MAX_DEVICES 600

typedef struct {
    char str[4];
} id_string_t;

BOOL id_string_eq(id_string_t* id_str, const char* str) {
    for (int i = 0; i < 3; i++) {
        if (id_str->str[i] != *(str + i))
            return FALSE;
    }
    return TRUE;
}

typedef struct {
    int* list;
    size_t len;
    size_t cap;
} connections_t;

#define DEFAULT_CAP 8
void connections_push(connections_t* conn, int id) {
    if (conn->cap == 0) {
        conn->list = (int*)malloc(sizeof(int) * DEFAULT_CAP);
        conn->cap = DEFAULT_CAP;
        conn->len = 0;
    } else if (conn->len >= conn->cap) {
        size_t new_cap = conn->cap * 2;
        conn->list = (int*)realloc((void*)conn->list, sizeof(int) * new_cap);
        conn->cap  = new_cap;
    }
    conn->list[conn->len++] = id;
}

static size_t n_devices;
static id_string_t devices[MAX_DEVICES];
static connections_t connections[MAX_DEVICES];
void reset_devices() {
    memset(devices, 0, sizeof(id_string_t) * MAX_DEVICES);
    n_devices = 0;
}

int find_id(const char* tok) {
    for (int i = 0; i < n_devices; i++) {
        id_string_t* id_str = &devices[i];
        if (id_string_eq(id_str, tok))
            return i;
    }
    return -1;
}

BOOL id_exists(const char* tok) {
    int id = find_id(tok);
    
    return id > -1;
}

size_t add_new_id(const char* tok) {
    assert(n_devices < MAX_DEVICES);
    id_string_t* id_str = &devices[n_devices++];
    memcpy(id_str->str, tok, 3);
    assert(id_str->str[3] == '\0');
    // printf("added new token: %s\n", id_str->str);
    return n_devices - 1;
}

#define BUF_SIZE 256
void read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FAILED TO OPEN FILE: %s\n", filename);
        exit(-1);
    }

    add_new_id("out");
    // first pass just get devices
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, file)) {
        const char* ptr = buf;
        // printf("%s\n", ptr);
        while (*ptr != '\0' && ptr != 0) {
            if (isalpha(*ptr)) {
                if (!id_exists(ptr)) {
                    add_new_id(ptr);
                }
                while (isalpha(*ptr)) ptr++;
            } else if (*ptr == ':') {
                break;
            } else {
                printf("Unknown token '%c'\n", *ptr);
                exit(-1);
            }
        }
    }

    fseek(file, 0, SEEK_SET);
    memset(buf, 0, BUF_SIZE);
    int n_outlets = 0;
    while (fgets(buf, BUF_SIZE, file)) {
        const char* ptr = buf;
        int device_id = find_id(buf);
        assert(device_id > 0);
        while (*ptr != ':') ptr++;
        ptr++;
        while (*ptr != '\0' && ptr != NULL) {
            if (isspace(*ptr)) {
                ptr++;
            } else if (isalpha(*ptr)) {
                int id = find_id(ptr);
                assert(id < MAX_DEVICES && id > -1);
                connections_push(&connections[device_id], id);
                if (id == 0) {
                    n_outlets++;
                }
                while (isalpha(*ptr)) ptr++;
            } else {
                printf("Unhandled token '%c'\n", *ptr);
                exit(-1);
            }
        }
    }
    printf("found %i outlets\n", n_outlets);
}

void connections_debug_print(connections_t* conn) {
    for (int i = 0; i < conn->len; i++) {
        printf("%i, ", conn->list[i]);
    }
    printf("\n");
}

int count_paths(int id, int* visited, int* memo) {
    if (id == 0) // found outlet
        return 1;

    if (memo[id] != 0) {
        return memo[id];
    }

    if (visited[id] != 0) { // avoid cycles
        // printf("Found cycle %s\n", devices[id].str);
        return 0;
    }
    visited[id] = 1;

    connections_t* conns = &connections[id];
    assert(conns != NULL && conns->list != NULL);
    int count = 0;
    for (int i = 0; i < conns->len; i++) {
        count += count_paths(conns->list[i], visited, memo);
    }
    // update the memoization table and remove the node from
    // the current path.
    memo[id] = count;
    visited[id] = 0;
    return count;
}

int part1(const char* filename) {
    reset_devices();
    read_file(filename);
    assert(find_id("you") > 0);
    int memo[MAX_DEVICES];
    int visited[MAX_DEVICES];
    memset(memo, 0, MAX_DEVICES * sizeof(int));
    memset(visited, 0, MAX_DEVICES * sizeof(int));
    return count_paths(find_id("you"), visited, memo);
}

void run_file(const char* filename) {
    printf("Running %s\n", filename);
    int n_paths = part1(filename);
    printf("Part 1: %i\n", n_paths);
}

int main() {
    run_file("sample.txt");
    // 1875635490 too high...
    run_file("input.txt");
}
