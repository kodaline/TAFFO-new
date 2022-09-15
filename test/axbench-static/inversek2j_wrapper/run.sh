#!/bin/bash
set -x
rm -rf data/output
mkdir -p data/output
benchmark=inversek2j
l1=(0.5 0.5 5 5 5 50 50)
l2=(0.5 5 0.5 5 50 5 50)
length=${#l1[@]}

if [ ! -d build ]
then
	mkdir build
	cd build
	cmake ..
	make
	cd ..
fi
for (( i=0; i<${length}; i++ ));
do 
	for f in data/input/*.data
	do
		filename=$(basename "$f")
		extension="${filename##*.}"
		filename="${filename%.*}"
	
		echo -e "\e[95m------ ${filename} ------\e[0m"
	
		echo -e "\e[96m*** Float Version ***\e[0m"
		time ./build/run_$benchmark ${f} data/output/${filename}_${benchmark}_${i}_out.data ${l1[$i]} ${l2[$i]}
	
	#echo -e "\e[96m*** Fix Version ***\e[0m"
	#time ./bin/inversek2j.out.fixp ${f} data/output/${filename}_${benchmark}_out.data.fixp
	
	#echo -e "\e[32m### QoS ###\e[0m"
	#python ./scripts/qos.py data/output/${filename}_${benchmark}_out.data data/output/${filename}_${benchmark}_out.data.fixp
	done
done