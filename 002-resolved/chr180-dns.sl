#!/bin/bash
#SBATCH --job-name=r180-dual-full
#SBATCH --time=12:00:00
#SBATCH -o slurm.out
#SBATCH -n 256
#SBATCH --account=ec231
#SBATCH --partition=standard
#SBATCH --qos=standard

source ~/.bashrc
module load valgrind
module load gcc/10.2.0
module load openmpi/4.1.4
cd $SLURM_SUBMIT_DIR
mpiexec ./vdf.x input.ptl -init 23000
