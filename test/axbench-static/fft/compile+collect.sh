#!/bin/bash

# $1 stats directory

if [[ $# -ne 1 ]]; then
  echo "usage: $0 <stats dir>"
  exit
fi

bench=fft
benchsrc=main

if [[ -z $DONT_REBUILD ]]; then
  make clean
  make all OX=-O3 DISABLE_ERROR=1
fi
cp stats/${benchsrc}.fixp.mix.txt $1/${bench}.imix.txt
cp stats/${benchsrc}.mix.txt $1/${bench}.float.imix.txt
cp stats/${benchsrc}.fixp.mlfeat.txt $1/${bench}.mlfeat.txt
cp stats/${benchsrc}.mlfeat.txt $1/${bench}.float.mlfeat.txt
cp stats/${benchsrc}.llvm.txt $1/${bench}.llstat.txt
#cat stats/errorprop.log | grep 'Computed error for target' > $1/${bench}.errprop.txt
