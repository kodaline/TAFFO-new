import sys
import os
import statistics

benchmarks = ["blackscholes", "blackscholes_00", "blackscholes_01", "fft", "fft_00", "fft_01", "fft_02", "inversek2j", "jmeint", "kmeans", "kmeans_00", "kmeans_01", "sobel"]
base_names = ["blackscholes", "blackscholes", "blackscholes", "fft", "fft_00", "fft_01", "fft_02", "inversek2j", "jmeint", "kmeans", "kmeans_00", "kmeans_01", "sobel"]

def chk_make(bench, bn):
    timefile = "maketime.txt"
    os.chdir(bench)
    os.system("make clean > /dev/null")
    os.system("rm -f " + timefile)
    os.system("/usr/bin/time -f \"%e\" -o " + timefile + " make " + bn + ".out > /dev/null 2> /dev/null")
    time = 0
    with open(timefile, 'r') as f:
        l = f.readline()
        time = float(l)
    os.chdir("..")
    return time

def chk_all(benchs, bn):
    times = []
    for i in range(len(benchs)):
        times.append(chk_make(benchs[i], bn[i]))
    return times

def chk_repeat(benchs, bn, n):
    table = []
    for i in range(n):
        table.append(chk_all(benchs, bn))
    return table

def compute_medians(table):
    meds = []
    for bench in range(len(table[0])):
        meds.append(statistics.median([table[i][bench] for i in range(len(table))]))
    return meds


if len(sys.argv) != 2:
  print("usage: %s <n_tries>" % sys.argv[0])
  exit(1)
ntries = int(sys.argv[1])

table = chk_repeat(benchmarks, base_names, ntries)
medians = compute_medians(table)

first_width = len("Median")
widths = [max(len(b)+1, 10) for b in benchmarks]
print("Trial " + "".join([(" %" + str(widths[i]-1) + "s") % benchmarks[i] for i in range(len(benchmarks))]))

for i in range(len(table)):
    print(("%" + str(first_width) + "d") % (i+1), end="")
    for j in range(len(table[i])):
        print(("%" + str(widths[j]) + ".6f") % table[i][j], end="")
    print("")

print("Median", end="")
for i in range(len(medians)):
    print(("%" + str(widths[i]) + ".6f") % medians[i], end="")
print("")

