#!/usr/bin/env python3

import sys
import os
import math
import statistics as stat


class Point(object):
    def __init__(self, coords):
        self.x = coords[0]
        self.y = coords[1]
        self.z = coords[2]
    def __add__(self, other):
        return Point((self.x + other.x, self.y + other.y, self.z + other.z))
    def __truediv__(self, other):
        return Point((self.x / other, self.y / other, self.z / other))
    def __iter__(self):
        return [self.x, self.y, self.z]


def c(p, q, r):
    return (p + q + r) / 3

def d(p, q):
    t1 = (p.x - q.x) ** 2
    t2 = (p.y - q.y) ** 2
    t3 = (p.z - q.z) ** 2
    return math.sqrt(t1+t2+t3)

def r(p, q, r):
    center = c(p, q, r)
    return max(d(center, p), d(center, q), d(center, r))

def compute_func(line):
    coords = list(map(lambda s: float(s), line.split()))
    abc_tri = (Point(coords[0:3]), Point(coords[3:6]), Point(coords[6:9]))
    def_tri = (Point(coords[9:12]), Point(coords[12:15]), Point(coords[15:18]))

    magnitude = max(coords)
    magnitude = math.ceil(2 * math.log(abs(magnitude), 100))

    distance = math.floor(min(1, (2 * d(c(*abc_tri), c(*def_tri)))/(r(*abc_tri)+r(*def_tri))))
    return distance + magnitude


if len(sys.argv) < 2:
    print("usage:", sys.argv[0], "dataset.txt")

for fn_data in sys.argv[1:]:
    classif_data = {}

    with open(fn_data, 'r') as f_data:
        f_data.readline() # throw away the number of pairs
        for line in f_data:
            classif = compute_func(line)
            try:
                thisdata = classif_data[classif]
            except:
                thisdata = []
                classif_data[classif] = thisdata
            thisdata.append(line)
                
    base, ext = os.path.splitext(fn_data)
    for key, val in classif_data.items():
        fn = base + '_batch' + str(key) + ext
        with open(fn, 'w') as fp:
            fp.write(str(len(val)) + '\n')
            fp.writelines(val)


