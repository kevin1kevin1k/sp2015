#!/bin/bash

size[0]=10000000
size[1]=1000000
size[2]=100000
size[3]=10000
size[4]=1000
size[5]=100
size[6]=10
size[7]=1

for i in {0..7}
do
	echo "size: 10000000, seg_size: ${size[$i]}"
	time ./merger ${size[$i]} < in10000000 > /dev/null
	echo "============================================"
done
