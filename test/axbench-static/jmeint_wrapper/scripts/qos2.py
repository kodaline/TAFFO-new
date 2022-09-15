#!/usr/bin/env python3

import sys
import statistics as stat


def get_one_triangle(file):
    res = {}
    line = file.readline()
    while line != '':
        if line.startswith('exit type'):
            return res
        parts = line.split()
        for part in parts:
            lhs, rhs = part.split('=')
            res[lhs] = float.fromhex(rhs)
        line = file.readline()
    return res


if len(sys.argv) < 2:
    print("usage:", sys.argv[0], "fix_1.txt float_1.txt fix_2.txt float_2.txt ...")

files_fix = sys.argv[1::2]
files_float = sys.argv[2::2]
if len(files_fix) != len(files_float):
    print("expected same number of float files and fix files")
    exit(1)

abs_error = {}

for fn_fix, fn_float in zip(files_fix, files_float):
    with open(fn_fix, 'r') as f_fix:
        with open(fn_float, 'r') as f_float:
            float_tri = get_one_triangle(f_float)
            fix_tri = get_one_triangle(f_fix)
            while float_tri != {} and fix_tri != {}:
                for key in float_tri:
                    try:
                        err_list = abs_error[key]
                    except:
                        err_list = []
                        abs_error[key] = err_list
                    n_float = float_tri[key]
                    try:
                        n_fix = fix_tri[key]
                        err_list.append(abs(n_float - n_fix))
                    except:
                        pass
                float_tri = get_one_triangle(f_float)
                fix_tri = get_one_triangle(f_fix)
            
for key, val in abs_error.items():
    print(key, ': mean =', stat.mean(val), ', median =', stat.median(val), ', max = ', max(val))


