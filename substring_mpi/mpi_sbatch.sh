#!/bin/bash -l
#SBATCH --job-name=substring_mpi

#SBATCH --mem=512M   # Memory per core, use --mem= for memory per node
#SBATCH --time=5:00   # Use the form MM:SS
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4

#SBATCH --output=substring_mpi.out
##SBATCH --constraint=dwarves

module load OpenMPI

mpirun /homes/brettnurnberg/project4/substring_mpi/substring_mpi /homes/dan/625/wiki_dump.txt 40