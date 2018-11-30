module load cuda/9.2.88
nvcc -dc pc4_v2.cu
nvcc -O -o lab4_arbitrary_v2 pc4_v2.o transform_cuda.o

time ./lab4_arbitrary_v2 < PC_data_t00100.dms>result_00100_v2.txt
