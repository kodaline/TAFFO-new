#!/bin/bash

OUTFILE="similarbits.txt"

# $1 benchmark to check
# $2 similarbits
check()
{
  pushd $1 > /dev/null
  echo "SIMILARBITS="$2 >> $OUTFILE
  export SIMILARBITS=$2
  make clean
  make all
  ./run2.sh >> $OUTFILE
  grep "Computed error for target" ./stats/errorprop.log >> $OUTFILE
  popd > /dev/null
}

# $1 benchmark to check
check_bench()
{
  rm ./$1/$OUTFILE
  for sb in {0..32}
  do
      check $1 $sb
  done
}

# enable bash logging
set -x

check_bench "blackscholes"
#check_bench "fft"
#check_bench "fft_00"
#check_bench "fft_02"
#check_bench "kmeans"
