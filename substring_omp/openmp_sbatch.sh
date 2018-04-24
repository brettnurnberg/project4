#!/bin/bash -l
#SBATCH --job-name=omp

#SBATCH --mem=512M
#SBATCH --nodes=1

#SBATCH --constraint=dwarves

COUNT=$1

export OMP_NUM_THREADS=$2

/homes/brettnurnberg/project4/substring_omp/substring_omp /homes/dan/625/wiki_dump.txt $COUNT