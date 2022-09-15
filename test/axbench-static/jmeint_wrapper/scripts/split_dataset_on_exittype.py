#!/usr/bin/env python3

import sys
import os
import statistics as stat


def get_one_triangle(file):
    res = {}
    line = file.readline()
    while line != '':
        if line.startswith('exit type'):
            if line.find('total') < 0:
                lhs, rhs = line.split('=')
                res['_exittype'] = rhs.strip()
            return res
        parts = line.split()
        for part in parts:
            lhs, rhs = part.split('=')
            res[lhs] = float.fromhex(rhs)
        line = file.readline()
    return res


if len(sys.argv) != 3:
    print("usage:", sys.argv[0], "log.txt dataset.txt")

fn_log = sys.argv[1]
fn_data = sys.argv[2]

classif_data = {}

with open(fn_log, 'r') as f_log:
    with open(fn_data, 'r') as f_data:
        f_data.readline() # throw away the number of pairs
        for line in f_data:
            classif = get_one_triangle(f_log)['_exittype']
            try:
                thisdata = classif_data[classif]
            except:
                thisdata = []
                classif_data[classif] = thisdata
            thisdata.append(line)
            
base, ext = os.path.splitext(fn_data)
for key, val in classif_data.items():
    fn = base + '_case' + str(key) + ext
    with open(fn, 'w') as fp:
        fp.write(str(len(val)) + '\n')
        fp.writelines(val)


