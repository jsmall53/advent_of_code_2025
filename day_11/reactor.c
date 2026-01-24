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
    memset(connections, 0, sizeof(connections_t) * MAX_DEVICES);
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
                while (isalpha(*ptr)) ptr++;
            } else {
                printf("Unhandled token '%c'\n", *ptr);
                exit(-1);
            }
        }
    }
}

void connections_debug_print(connections_t* conn) {
    for (int i = 0; i < conn->len; i++) {
        printf("%i, ", conn->list[i]);
    }
    printf("\n");
}

int count_paths_dfs(int id, int dst, int* visited, int* reachable) {
    if (id == dst) {// found outlet
        return 1;
    }
    visited[id] = 1;
    connections_t* conns = &connections[id];
    int count = 0;
    if (conns->len > 0) {
        for (int i = 0; i < conns->len; i++) {
            int next_id = conns->list[i];
            if (visited[next_id] == 0 && reachable[next_id]) {
                count += count_paths_dfs(next_id, dst, visited, reachable);
            }
        }
    }
    // remove the node from the current path.
    visited[id] = 0;
    return count;
}


void path_debug_print(int* visited) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (visited[i] == 1) {
            printf("%s,", devices[i].str);
        }
    }
    printf("\n");
}

int can_reach(int id, int dst, int* visited) {
    if (id == dst) {
        return 1;
    }

    if (visited[id] != 0) {
        return 0;
    }

    visited[id] = 1;
    connections_t* conns = &connections[id];
    int count = 0;
    for (int i = 0; i < conns->len; i++) {
        if (can_reach(conns->list[i], dst, visited) > 0) {
            return 1;
        }
    }
    return 0;
}

void find_reachable(int id, int dst, int* visited, int* reachable) {
    if (id == dst) {
        reachable[id] = 1;
        return;
    }

    if (reachable[id] == 1)
        return;

    int visited2[MAX_DEVICES] = {0};
    reachable[id] = can_reach(id, dst, visited2);
    visited[id] = 1;
    connections_t* conns = &connections[id];
    int count = 0;
    for (int i = 0; i < conns->len; i++) {
        if (visited[conns->list[i]] == 0) {
            find_reachable(conns->list[i], dst, visited, reachable);
        }
    }
    return;

}

void visited_dbg_print(int* visited) {
    int count = 0;
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (visited[i] == 1)
            printf("%s,", devices[i].str);
        count += visited[i];
    }
    printf(" visited %i vertices\n", count);
}


int part1(const char* filename) {
    reset_devices();
    read_file(filename);
    assert(find_id("you") > 0);
    int reachable[MAX_DEVICES] = {0};
    int visited[MAX_DEVICES] = {0};
    find_reachable(find_id("you"), find_id("out"), visited, reachable);
    // visited_dbg_print(reachable);
    // memset(reachable, 1, MAX_DEVICES * sizeof(int));
    memset(visited, 0, MAX_DEVICES * sizeof(int));
    return count_paths_dfs(find_id("you"), find_id("out"), visited, reachable);
}

void explore(int id, int* visited) {
    visited[id] = 1;
    connections_t* conns = &connections[id];
    for (int i = 0; i < conns->len; i++) {
        if (visited[i] == 0)
            explore(conns->list[i], visited);
    }
    return;
}

// 150 too low
// 5392 too low
// 10006182 too low
size_t part2(const char* filename) {
    reset_devices();
    read_file(filename);
    assert(find_id("svr") > 0 && find_id("dac") > 0 && find_id("fft") > 0);

    int svr = find_id("svr");
    int fft = find_id("fft");
    int dac = find_id("dac");
    int out = 0;

    int visited[MAX_DEVICES] = {0};
    assert(!can_reach(dac, fft, visited));
    memset(visited, 0, sizeof(int) * MAX_DEVICES);

    int reachable_set[MAX_DEVICES] = {0};
    size_t total = 1;

    /* Addition of each path segment didn't work 
     * Its multiplication, duh.*/
    find_reachable(svr, fft, visited, reachable_set);
    memset(visited, 0, sizeof(int) * MAX_DEVICES);
    total *= count_paths_dfs(svr, fft, visited, reachable_set);
    printf("svr->fft: %zu\n", total);

    memset(reachable_set, 0, sizeof(int) * MAX_DEVICES);
    memset(visited, 0, sizeof(int) * MAX_DEVICES);
    find_reachable(fft, dac, visited, reachable_set);
    memset(visited, 0, sizeof(int) * MAX_DEVICES);
    total *= count_paths_dfs(fft, dac, visited, reachable_set);
    printf("svr->fft->dac: %zu\n", total);

    memset(reachable_set, 0, sizeof(int) * MAX_DEVICES);
    memset(visited, 0, sizeof(int) * MAX_DEVICES);
    find_reachable(dac, out, visited, reachable_set);
    memset(visited, 0, sizeof(int) * MAX_DEVICES);
    total *= count_paths_dfs(find_id("dac"), find_id("out"), visited, reachable_set);
    printf("svr->fft->dac->out: %zu\n", total);

    return total;
}



void run_file(const char* filename) {
    printf("\nRunning %s\n", filename);
    int n_paths = part1(filename);
    printf("Part 1: %i\n", n_paths);
    size_t paths = part2(filename);
    printf("Part 2: %zu\n", paths);
}

int main() {
    part1("sample.txt");
    printf("Sample part 2: %zu\n", part2("sample2.txt"));

    run_file("input.txt");
}
