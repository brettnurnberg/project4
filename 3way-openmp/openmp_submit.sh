#!/bin/bash -l

for i in 1 2 3 4 5 6 7 8 9 10
do
	for j in 1 2 4 8 16 32
	do
		for k in 1000 10000 100000 500000 1000000
		do
			sbatch --time=00:$((k/(j*16)+1800)) --ntasks-per-node=$j openmp_sbatch.sh $k $j
		done
	done
done
