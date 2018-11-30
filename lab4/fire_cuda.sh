module load cuda/9.2.88
nvcc -dc pc4.cu
nvcc -O -o lab4_arbitrary pc4.o transform_cuda.o

time ./lab4_arbitrary < PC_data_t00100.dms>result_100.txt
