#!/bin/bash -l
#SBATCH --job-name=substring_pthread

#SBATCH --mem-per-cpu=512M   # Memory per core, use --mem= for memory per node
#SBATCH --time=10:00   # Use the form MM:SS
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4

#SBATCH --output=substring_pthread.out
#SBATCH --constraint=dwarves

/homes/brettnurnberg/project4/substring_pthread/substring_pthread /homes/dan/625/wiki_dump.txt 1000