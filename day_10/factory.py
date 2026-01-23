from scipy.optimize import linprog, milp, LinearConstraint, Bounds

MAX_LIGHTS = 10
n_devices = 0
buttons   = []
joltage_targets = []

def parse_num_list(button_str, bracket_open, bracket_close):
    button = []
    assert(button_str[0] == bracket_open)
    assert(button_str[len(button_str) - 1] == bracket_close)
    parts = button_str[1:len(button_str) - 1].split(',')
    for part in parts:
        button.append(int(part))
    return button


def read_file(filename: str):
    with open(filename, 'r') as f:
        for line in f:
            parts = line.split(' ')
            device_buttons = []
            for part in parts:
                if part[0] == '[':
                    continue
                elif part[0] == '(':
                    device_buttons.append(parse_num_list(part, '(', ')'))
                elif part[0] == '{':
                    joltage_targets.append(parse_num_list(part.strip(), '{', '}'))
                else:
                    pass
            buttons.append(device_buttons)
            global n_devices
            n_devices += 1
            assert(len(buttons) == n_devices and len(joltage_targets) == n_devices)

def objective_fn(i: int):
    return[1] * len(buttons[i])

def bounds(i: int):
    return [(0, 512) for _ in range(len(buttons[i]))]

def constraints_a(i: int):
    # I need MAX_LIGHTS total lists. Each list represents a joltage pos. Each list will be n_buttons long
    matrix = []
    for k in range(MAX_LIGHTS):
        coeff = []
        for button in buttons[i]:
            if k in button:
                coeff.append(1)
            else:
                coeff.append(0)
    # for button in buttons[i]:
    #     coeff = [0] * MAX_LIGHTS
    #     for b in button:
    #         coeff[b] = 1
        matrix.append(coeff)
    return matrix

def constraints_b(i: int):
    fn = []
    for k in range(MAX_LIGHTS):
        if k < len(joltage_targets[i]):
            fn.append(joltage_targets[i][k])
        else:
            fn.append(0)
    return fn

# 16430 too low
# 16440 too low
if __name__ == "__main__":
    # read_file("sample.txt")
    read_file("input.txt")
    # print(objective_fn(0))
    # print(bounds(0))
    # print(constraints_a(0))
    # print(constraints_b(0))
    total = 0
    for i in range(n_devices):
        eq_cons = LinearConstraint(constraints_a(i), constraints_b(i), constraints_b(i))
        bounds = Bounds(0, 512)
        integrality = [1 for _ in range(len(buttons[i]))]
        res = milp(c=objective_fn(i), constraints=[eq_cons], bounds=bounds, integrality=integrality)
        # res = linprog(
        #     c = objective_fn(i),
        #     bounds = bounds(i),
        #     A_eq=constraints_a(i),
        #     b_eq=constraints_b(i),
        #     )
        total += res.fun
        if i == 70:
            print(res)
    print(f"{total} presses.")
