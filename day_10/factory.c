#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#define LIGHT_OFF_CHAR '.'
#define LIGHT_OFF       0
#define LIGHT_ON_CHAR  '#'
#define LIGHT_ON        1

#define MAX_BUTTONS 32
#define MAX_LIGHTS 10
#define MAX_STATES (2 << MAX_LIGHTS)
#define JOLTAGE_MAX 1000

typedef struct {
    unsigned int value;
    int n;
} lights_t;

typedef struct {
    unsigned int* list;
    size_t cap;
    size_t len;
} button_list_t;

typedef struct {
    unsigned int vals[MAX_LIGHTS];
    int n;
} joltage_array_t;

#define N 151
static int n_machines;
static lights_t indicator_lights_target[N];
static button_list_t machine_buttons[N];
static joltage_array_t joltages[N];
static unsigned int transitions[MAX_STATES][MAX_STATES];

void joltage_array_push(joltage_array_t* arr, unsigned int val) {
    assert(arr->n < MAX_LIGHTS);
    arr->vals[arr->n++] = val;
}

void lights_print(lights_t* lights) {
    fprintf(stdout, "[");
    for (int i = 0; i < lights->n; i++) {
        char c = (lights->value >> i) & 0x1 ? LIGHT_ON_CHAR : LIGHT_OFF_CHAR;
        fprintf(stdout, "%c", c);
    }
    fprintf(stdout, "]\n");
}

void lights_push(lights_t* lights, char c) {
    if (lights->n >= MAX_LIGHTS) {
        fprintf(stderr, "Indicator light already at max lights.\n");
        assert(0);
    }
    lights->value |= c << lights->n++;
}

#define DEFAULT_CAP 8
void button_list_init(button_list_t* buttons) {
    buttons->len = 0;
    buttons->cap = DEFAULT_CAP;
    buttons->list = (unsigned int*)malloc(sizeof(unsigned int) * DEFAULT_CAP);
}

void button_list_reset(button_list_t* buttons) {
    if (buttons->list)
        free(buttons->list);
    button_list_init(buttons);

}

void button_list_push(button_list_t* buttons, unsigned int mask) {
    if (buttons->len >= buttons->cap) {
        size_t new_size = buttons->cap * 2;
        buttons->list = (unsigned int*)realloc(buttons->list, sizeof(unsigned int) * new_size);
        buttons->cap = new_size;
    }
    buttons->list[buttons->len++] = mask;
}

void reset_state() {
    n_machines = 0;
    memset(indicator_lights_target, 0, sizeof(*indicator_lights_target) * N);
    // can free all these if I want.
    memset(machine_buttons, 0, sizeof(*machine_buttons) * N);
    memset(joltages, 0, sizeof(*joltages) * N);
}

unsigned int state_toggle_bit(unsigned int state, int bit_index) {
    unsigned int mask = 0x1 << bit_index;
    if ((state >> bit_index) & 0x1) {
        state &= ~mask;
    } else {
        state |= mask;
    }
    return state;
}

unsigned int state_apply_button(unsigned int state, unsigned int mask) {
    int index = 0;
    while ((mask >> index )> 0) {
        if ((mask >> index) & 0x1) {
            state = state_toggle_bit(state, index);
        }
        index++;
    }
    return state;
}

void build_transition_table() {
    for (int state = 0; state < MAX_STATES; state++) {
        for (int button_val = 0; button_val < MAX_STATES; button_val++) {
            transitions[state][button_val] = state_apply_button(state, button_val);
        }
    }
}

void parse(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("failed to open input file: %s\n", filename);
        exit(-1);
    }

    char buf[4096];
    while (fgets(buf, 4096, file)) {
        char* c = buf;
        int eof_found = 0;
        button_list_reset(&machine_buttons[n_machines]);
        while (*c != '\n' && *c != '\0' && !eof_found) {
            while (isspace(*c)) {
                c++;
            }

            switch (*c) {
                case '[':
                {
                    c++;
                    lights_t* lights = &indicator_lights_target[n_machines];
                    while (*c != ']') {
                        if (*c == '.') {
                            lights_push(lights, LIGHT_OFF);
                        } else if (*c == '#') {
                            lights_push(lights, LIGHT_ON);
                        } else {
                            fprintf(stderr, "Unexpected token during indicator light parse '%c'\n", *c);
                            exit(-1);
                        }
                        c++;
                    }
                    c++;
                    break;
                }
                case '(':
                {
                    c++;
                    unsigned int mask = 0;
                    while (*c != ')') {
                        if (isdigit(*c)) {
                            // Should be single digit 0-9
                            char index = *c - '0';
                            mask |= 0x1 << index;
                        } else if (*c == ',') {
                            // NOOP
                        } else {
                            fprintf(stderr, "Unexpected token during button parse '%c'\n", *c);
                            exit(-1);
                        }
                        c++;
                    }
                    button_list_push(&machine_buttons[n_machines], mask);
                    c++;
                    break;
                }
                case '{':
                {
                    c++;
                    unsigned int val = 0;
                    int index = 0;
                    while (*c != '}') {
                        if (isdigit(*c)) {
                            val = val * 10 + (*c - '0');
                        } else if (*c == ',') {
                            joltage_array_push(&joltages[n_machines], val);
                            val = 0;
                        } else {
                            fprintf(stderr, "Unexpected token during joltage parse '%c'\n", *c);
                            exit(-1);
                        }
                        c++;
                    }
                    joltage_array_push(&joltages[n_machines], val);
                    assert(joltages[n_machines].n == indicator_lights_target[n_machines].n);
                    c++;
                    break;
                }
                case '\0':
                    break;
                default:
                    fprintf(stderr, "undhandled token: %c\n", *c);
                    break;
            }
        }
        n_machines++;
    }
}



/*
 * Is this is a finite state machine problem?
 * - How could I use a finite state machine to
 *
 * For every light state, map a resulting state for each button press
 * */
#define MAX_PRESSES 512

typedef struct {
    size_t id;
    unsigned int state[MAX_LIGHTS];
    unsigned int next[MAX_BUTTONS][MAX_LIGHTS];
} joltage_node_t;

typedef struct {
    joltage_node_t* nodes;
    size_t cap;
    size_t len;
} node_list_t; 

void node_list_init(node_list_t* list, size_t cap) {
    list->len = 0;
    list->cap = cap;
    list->nodes = (joltage_node_t*)malloc(sizeof(joltage_node_t) * cap);
}

void node_list_free(node_list_t* list) {
    free(list->nodes);
    list->nodes = 0;
    list->cap = 0;
    list->len = 0;
}

int node_equal(joltage_node_t* node, unsigned int* state) {
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (node->state[i] != state[i])
            return 0;
    }
    return 1;
}

joltage_node_t* node_list_push(node_list_t* list, unsigned int* state) {
    if (list->len >= list->cap) {
        size_t new_size = list->cap * 2;
        list->nodes = (joltage_node_t*)realloc(list->nodes, sizeof(joltage_node_t) * new_size);
        list->cap = new_size;
    }
    size_t node_id = list->len++;
    joltage_node_t* node = &list->nodes[node_id];
    node->id = node_id;
    memcpy(node->state, state, sizeof(*state) * MAX_LIGHTS);
    return node;
}


typedef struct queue_node queue_node_t;
typedef struct queue_node {
    unsigned int state;
    queue_node_t* next;
} queue_node_t;

queue_node_t* queue_node_create(unsigned int state) {
    queue_node_t* node = (queue_node_t*)malloc(sizeof(queue_node_t));
    node->state = state;
    node->next = NULL;
    return node;
}

typedef struct queue {
    queue_node_t* head;
    size_t n;
} queue_t;

void queue_push(queue_t* queue, unsigned int state) {
    if (queue->head == NULL && queue->n == 0) {
        queue->head = queue_node_create(state);
        queue->n++;
    } else {
        queue_node_t* last = queue->head;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = queue_node_create(state);
        queue->n++;
    }
}

unsigned int queue_pop(queue_t* queue) {
    assert(queue->head != NULL);
    queue_node_t* cur = queue->head;
    queue->head = queue->head->next;
    unsigned int val = cur->state;
    free(cur);
    cur = NULL;
    queue->n--;
    return val;
}

void test_queue() {
    queue_t queue = {0};
    queue_push(&queue, 123);
    assert(queue.head->state == 123);
    assert(queue.n == 1);
    assert(123 == queue_pop(&queue));
    assert(queue.n == 0);
    queue_push(&queue, 1234);
    assert(queue.n == 1);
    queue_push(&queue, 5678);
    assert(queue.n == 2);
    assert(1234 == queue_pop(&queue));
    assert(queue.n == 1);
    assert(5678 == queue_pop(&queue));
    assert(queue.n == 0);
}

// Probably need breadth first search.
unsigned int machine_find_lights(int machine_id) {
    unsigned int target = indicator_lights_target[machine_id].value;
    int n = indicator_lights_target[machine_id].n;
    button_list_t* buttons = &machine_buttons[machine_id];

    unsigned int presses[MAX_STATES] = {0};
    unsigned int visited[MAX_STATES] = {0};
    queue_t queue = {0};
    queue_push(&queue, 0);
    while (queue.n > 0) {
        unsigned int state = queue_pop(&queue);
        if (state == target) {
            // printf("FOUND TARGET STATE (%#b) FOR MACHINE: %i\n", target, machine_id);
            return presses[state];
        }
        for (int i = 0; i < buttons->len; i++) {
            unsigned int next = transitions[state][buttons->list[i]];
            if (visited[next] == 0) {
                visited[next]++;
                presses[next] = presses[state] + 1;
                queue_push(&queue, next);
            }
        }
    }

    return 0;
}

// State is now an array. How does this change the BFS solution?
// Need a lot more storage. This might require recursive BFS?
unsigned int machine_find_joltages(int machine_id) {
    joltage_array_t* target = &joltages[machine_id];
    button_list_t* buttons = &machine_buttons[machine_id];
    node_list_t list = {0};


    return 0;
}

int part1(const char* filename) {
    reset_state();
    parse(filename);
    int result = 0;
    for (int i = 0; i < n_machines; i++) {
        result += machine_find_lights(i);
    }
    return result;
}

void part2(const char* filename) {
    reset_state();
    parse(filename);
}

void test_sample() {
    reset_state();
    parse("sample.txt");
    // printf("%b\n", transitions[0b0000000000][0b0000000101]);
    assert(state_toggle_bit(0b0000000000, 0) == 1);
    assert(state_toggle_bit(0b0000000000, 1) == 2);
    assert(state_toggle_bit(0b0000000000, 2) == 4);
    assert(state_toggle_bit(0b0000000000, 3) == 8);
    assert(state_toggle_bit(0b0000000000, 4) == 16);

    assert(state_apply_button(0b0, 0b1) == 1);
    assert(state_apply_button(0b0, 0b10) == 2);
    assert(state_apply_button(0b0, 0b11) == 3);
    unsigned int state = 0b0110;
    state = state_apply_button(state, 0b11);
    state = state_apply_button(state, 0b101);

    assert(transitions[0][0] == 0);
    assert(transitions[0][1] == 1);
    assert(state_apply_button(0, 1023) == 1023);
    assert(transitions[0][1023] == 1023);
    assert(transitions[1023][0] == 1023);
    assert(transitions[22][512] == transitions[512][22]);
}

int main() {
    build_transition_table();

    // assert(part1("sample.txt") == 7);
    // printf("Part 1 resut: %i\n", part1("input.txt"));

    part2("sample.txt");
    joltage_state state = joltage_state_from_arr(&joltages[0]);
    assert(state.id == 7004005003);
    joltage_state state2 = joltage_state_apply_button(&state, machine_buttons[0].list[0]);
    printf("%zu\n", state2.id);
    assert(state2.id == 8004005003);
}
