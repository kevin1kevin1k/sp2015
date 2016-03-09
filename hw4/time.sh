#!/bin/bash

size[0]=100
size[1]=10000
size[2]=1000000
size[3]=10000000

nseg[0]=100
nseg[1]=25
nseg[2]=10
nseg[3]=5
nseg[4]=2
nseg[5]=1

for i in {0..3}
do
	for j in {0..5}
	do
		echo "size: ${size[$i]}, seg_size: $((${size[$i]}/${nseg[$j]}))"
		time ./merger $((${size[$i]}/${nseg[$j]})) < in${size[$i]} > /dev/null
		echo "============================================"
	done
done
