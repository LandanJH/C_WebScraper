#!/bin/sh

gcc final_seq.c -o seq.bin -lcurl -fopenmp
mpicc final_mpi.c -o mpi.bin -lcurl
gcc final_omp.c -o omp.bin -lcurl -fopenmp