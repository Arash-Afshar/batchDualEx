
import sys
from math import log

#n = int(sys.argv[0])
#lg = log(n, 2)
max_mem = int(sys.argv[1])
word = int(sys.argv[2])
ratio = 4
C = 1


def extract_gate_from_file(path):
    with open(path, "r") as f:
        #gate = int(f.readline().split(" ")[0].strip())
        f.readline()
        f.readline()
        f.readline()
        gate = int(f.readline().strip())
    return gate

def get_ui_size(mem_size, elem_size):
    return extract_gate_from_file("./universalInstruction-" + str(mem_size) + "-" + str(elem_size) + ".circ")

def get_halt_size(elem_size):
    return elem_size * 4

def get_recursive_rw_size(addr_size):
    if addr_size <= 1:
        return 0, 0, 0
    mem_size = 2 ** addr_size
    rw_asym = C * 4 * (2 * addr_size + (2 * addr_size) + 1) * (addr_size + 1) + addr_size + addr_size + (2 * addr_size) + 1
    evict_asym = 3 * C * (4 * (2 * (addr_size) + addr_size * 2 + 1) * (addr_size + 1) + addr_size)

    rw = extract_gate_from_file("./readWrite-" + str(int(mem_size)) + "-" + str(int(addr_size)) + ".circ")
    evict = 2 * extract_gate_from_file("./evict-" + str(int(mem_size)) + "-" + str(int(addr_size)) + ".circ")
    recurs, recurs_asym, recurs_semi_asym = get_recursive_rw_size(log(mem_size / ratio, 2))


    return rw + evict + recurs, rw_asym + evict_asym + recurs_asym, rw + evict_asym + recurs_semi_asym


def get_rw_size(mem_size, addr_size, elem_size):
    main_rw_asym = C * 4 * (2 * addr_size + elem_size + 1) * (addr_size + 1) + addr_size + addr_size + elem_size + 1
    main_evict_asym = 3 * C * (4 * (2 * (addr_size) + elem_size + 1) * (addr_size + 1) + addr_size)

    main_rw = extract_gate_from_file("./readWrite-" + str(int(mem_size)) + "-" + str(int(elem_size)) + ".circ")
    main_evict = extract_gate_from_file("./evict-" + str(int(mem_size)) + "-" + str(int(elem_size)) + ".circ")

    recursive_rw, recursive_rw_asym, recursive_rw_semi_asym = get_recursive_rw_size(log(mem_size / ratio, 2))

    return main_rw + 2 * main_evict + recursive_rw, main_rw_asym + 2 * main_evict_asym + recursive_rw_asym, main_rw + 2 * main_evict_asym + recursive_rw_semi_asym

def calc_gate_count(mem_size, addr_size, elem_size):
    ui = get_ui_size(mem_size, elem_size)
    rw, rw_asym, rw_semi_asym = get_rw_size(mem_size, addr_size, elem_size)
    hlt = get_halt_size(elem_size)

    total = ui + hlt + 4 * rw
    total_asym = ui + hlt + 4 * rw_asym
    total_semi_asym = ui + hlt + 4 * rw_semi_asym

    return total, total_asym, total_semi_asym


def estimate_gate_count(mem_size):
    n = mem_size
    lg = log(n, 2)
    T=20*lg
    estimate = (T**3) * (log(T, 2))
    return estimate
    #estiimate=`echo "n=${n}; lg=l(n)/l(2); a=8000 *(lg^3); b=(l(20 * lg)/l(2)); scale=0; a*b/1" | bc -l`

def get_phi(total_T):
    phi = 20
    if total_T >= 500000:
        phi = 5
    elif total_T >= 100000:
        phi = 7
    elif total_T >= 500:
        phi = 9
    elif total_T >= 250:
        phi = 11
    elif total_T >= 100:
        phi = 13
    return phi


for i in xrange(2, max_mem + 1):
    n = 2 ** i
    lg = i

    actual_cpu_step, actual_cpu_step_asym, actual_cpu_step_semi_asym = calc_gate_count(n, lg, word)
    est_computation = C * int(estimate_gate_count(n))


    T = 20 * lg

    Q = 100
    phi = get_phi(T * Q)
    res = 2 * phi * (Q * T * actual_cpu_step)
    res_asym = 2 * phi * (Q * T * actual_cpu_step_asym)
    res_semi_asym = 2 * phi * (Q * T * actual_cpu_step_semi_asym)

    est = 40 * (Q * est_computation)

    M = 1
    Q = Q / M
    phi = get_phi(Q)
    est = 2 * phi * (Q * M * est_computation)


    print(str(n) + " " + str(res) + " " + str(res_asym) + " " + str(res_semi_asym) + " " + str(est))

