#!/bin/bash

rm -rf data/output
mkdir -p data/output
benchmark=inversek2j

for f in ../inversek2j_00/data/input/*.data
do
  i=0
  for l12 in '0.5 0.5' '0.5 5' '5 0.5' '5 5' '5 50' '50 5' '50 50'
  do
	filename=$(basename "$f")
	extension="${filename##*.}"
	filename="${filename%.*}"
	
	echo -e "\e[95m------ ${filename} 0$i ------\e[0m"
	
	echo -e "\e[96m*** Float Version ***\e[0m"
	time ./bin/inversek2j.out ${f} data/output/${filename}_${benchmark}_out.data $l12
	
	echo -e "\e[96m*** Fix Version ***\e[0m"
	time ./bin/inversek2j.out.fixp ${f} data/output/${filename}_${benchmark}_out.data.fixp $l12
	
	echo -e "\e[32m### QoS ###\e[0m"
	python ./scripts/qos.py data/output/${filename}_${benchmark}_out.data data/output/${filename}_${benchmark}_out.data.fixp

	i=$((i+1))
  done
done
