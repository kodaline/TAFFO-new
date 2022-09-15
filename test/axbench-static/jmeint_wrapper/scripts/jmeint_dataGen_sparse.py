#!/usr/bin/python


import sys
import random


# python jmeint_dataGen.py <number of items> <output file>

outFile = open(sys.argv[2], "w")

outFile.write("%d\n" % (int(sys.argv[1])))

for i in range(int(sys.argv[1])):
  for j in range(18/(3*3)):
    center = random.uniform(0.01, 1.0 - 0.01)
    for k in range(3*3):
      currValue = random.uniform(0, 0.01)
      outFile.write("%f\t" % (currValue + center))
  outFile.write("\n")

outFile.close()
