#!/bin/bash -l

for i in 1 2 3 4 5 6 7 8 9 10
do
	for k in 1000 10000 100000 500000 1000000
	do
		for j in 1 2 4 8 16 32
		do
			sbatch --time=00:$((k/(j*16)+1800)) --ntasks-per-node=$j --nodes=1 mpi_sbatch.sh $k
		done
		
		sbatch --time=00:$((k/(6*16)+1800)) --ntasks-per-node=4 --nodes=2 mpi_sbatch.sh $k
		sbatch --time=00:$((k/(10*16)+1800)) --ntasks-per-node=4 --nodes=4 mpi_sbatch.sh $k
		sbatch --time=00:$((k/(20*16)+1800)) --ntasks-per-node=16 --nodes=2 mpi_sbatch.sh $k
		sbatch --time=00:$((k/(10*16)+1800)) --ntasks-per-node=2 --nodes=16 mpi_sbatch.sh $k
		
	done
done
