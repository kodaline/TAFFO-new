#!/bin/bash

rm -rf data/output
mkdir data/output
benchmark=jmeint

# decompress data files
for f in data/input/*.data.bz2; do
  uncompressed="${f%.bz2}"
  if [[ ! ( -e ${uncompressed} ) ]]; then
    bunzip2 -k ${f}
  fi
done
# cleanup stale data files
for f in data/input/*.data; do
  compressed="${f}.bz2"
  if [[ ! ( -e ${compressed} ) ]]; then
    rm ${f}
  fi
done

for f in data/input/*.data
do
	filename=$(basename "$f")
	extension="${filename##*.}"
	filename="${filename%.*}"
	
	echo -e "\e[95m------ ${filename} ------\e[0m"
	
	echo -e "\e[96m*** Float Version ***\e[0m"
	time ./bin/${benchmark}.out ${f} data/output/${filename}_${benchmark}_out.data 2> log_${filename}_float.txt
	
	echo -e "\e[96m*** Fix Version ***\e[0m"
	time ./bin/${benchmark}.out.fixp ${f} data/output/${filename}_${benchmark}_out.data.fixp 2> log_${filename}_fixp.txt
	
	echo -e "\e[32m### QoS ###\e[0m"
	python ./scripts/qos.py data/output/${filename}_${benchmark}_out.data data/output/${filename}_${benchmark}_out.data.fixp
done
