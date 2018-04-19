#!/bin/bash -l
#SBATCH --job-name=substring_omp

#SBATCH --mem=512M   # Memory per core, use --mem= for memory per node
#SBATCH --time=1:00   # Use the form MM:SS
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4

#SBATCH --output=debug_substring_omp.out
##SBATCH --constraint=dwarves

export OMP_NUM_THREADS=4

/homes/brettnurnberg/project4/substring_omp/substring_omp /homes/dan/625/wiki_dump.txt 40