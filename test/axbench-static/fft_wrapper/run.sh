#!/bin/bash

rm -rf data/output
mkdir -p data/output
benchmark=fft_00

input=( 2048 8192 65536 )

for f in "${input[@]}"
do
	
	echo -e "\e[95m------ ${f} ------\e[0m"
	
	echo -e "\e[96m*** Float Version ***\e[0m"
	time ./bin/run_fft ${f} data/output/${f}_${benchmark}_out.data
	
	echo -e "\e[96m*** Fix Version ***\e[0m"
	time ./bin/${benchmark}.out.fixp ${f} data/output/${f}_${benchmark}_out.data.fixp
	
	echo -e "\e[32m### QoS ###\e[0m"
	python ./scripts/qos.py data/output/${f}_${benchmark}_out.data data/output/${f}_${benchmark}_out.data.fixp
done
