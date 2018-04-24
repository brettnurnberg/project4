#!/bin/bash -l
#SBATCH --job-name=pthread

#SBATCH --mem=512M   # Memory per core, use --mem= for memory per node
#SBATCH --nodes=1

#SBATCH --constraint=dwarves

COUNT=$1

/homes/brettnurnberg/project4/substring_pthread/substring_pthread /homes/dan/625/wiki_dump.txt $COUNT