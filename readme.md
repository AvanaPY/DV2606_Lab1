# Lab 1

## Running the program with make

`make compile_all` to compile all the programs into executables

`make clean` to clean the directory

# Executables

`gaussian_sequential.a` is the sequential version of the gaussian elimination, equivalent to the one given to us in the assignment.

`qsortseq.a` is the sequential version of the quick sort algorithm, equivalent to the one given to us in the assignment.

`gaussian_parallel.a` is the parallel version of gaussian elimination, by default uses 16 threads and a 2048x2048 matrix.

`qsortpar.a` is the parallel version of the quick sort algorithm, by default uses 16 threads with 64*MEGA items.