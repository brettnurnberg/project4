#!/bin/bash -l
#SBATCH --job-name=mpi

#SBATCH --mem=1G   # Memory per core, use --mem= for memory per node

#SBATCH --constraint=dwarves

COUNT=$1

module load OpenMPI

mpirun /homes/brettnurnberg/project4/substring_mpi/substring_mpi /homes/dan/625/wiki_dump.txt $COUNT