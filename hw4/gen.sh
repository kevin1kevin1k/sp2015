#!/bin/bash

size[0]=100
size[1]=10000
size[2]=1000000
size[3]=10000000

for i in {0..3}
	do ./rand ${size[$i]} > in${size[$i]}
done
