/*
 * Author: Deren Kong
 * Description: simple hierarchical parallelism program using openmp and MVAPICH2
 * compile: make
 * submit date: 12/05/2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <string.h>
#include <time.h>
#include "pc5.h"
#define UINT_MAX 65535
#define NUM_THREAD 4
// work_queue initiaition and operations
queue *init_queue() {
    queue * queue_list;
    queue_list = (queue*) malloc (sizeof(queue));
    int queue_size = N;
    // initialize the work_queue with ' ' for cmd and UINT_MAX for key
    for (int i = 0; i < N; i++) {
        queue_list->work_queue[i].cmd = ' ';
        queue_list->work_queue[i].key = UINT_MAX;
    }
    queue_list->empty = 1;
    queue_list->full = 0;
    queue_list->head = 0;
    queue_list->tail = 0;
    return queue_list;
}

void destroy_queue(queue *q) {
    free(q);
}

void add_queue(queue *q, work_entry element) {
    q->work_queue[q->tail] = element;
    q->tail++;
    if (q->tail == N) {
        q->tail = 0;
    }
    if (q->tail == q->head) {
        q->full = 1;
    }
    q->empty = 0;
}

void del_queue(queue *q, work_entry *out) {
    *out = q->work_queue[q->head];
    q->head++;
    if (q->head == N) {
        q->head = 0;
    }
    if (q->head == q->tail) {
        q->empty = 1;
    }
    q->full = 0;
}

int main(int argc, char** argv) {
	int workers, rank;
	int provided;
	MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE, &provided);
	if(provided != MPI_THREAD_MULTIPLE)
	    {
	        printf("MPI do not Support Multiple thread\n");
	        exit(0);
	    }
	MPI_Comm_size(MPI_COMM_WORLD, &workers);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int iter = 0;
	int flag = 0;

	double time_p, time_c, time_t;
	clock_t c_p, c_c, c_t;
//master process begin to work
	if (rank == 0) {
	queue * queue_list = init_queue();
	if (queue_list == NULL) {
	    printf("Failed to init!\n");
	}    
	char buffer[20];
	char temp_key[10];
	char cmd = ' ';
	while(fgets(buffer, 20, stdin) != NULL) {
	    sscanf(buffer, "%c  %s", &cmd, temp_key);
	    u_int16_t key = (u_int16_t)(atoi(temp_key));
	    work_entry buf;
	    if (cmd == 'X') {
	        break;
	    }
	    if (cmd != 'A' && cmd != 'B' && cmd != 'C' && cmd != 'D') {
	        continue;
	    }
	    if (key <= 1000) {
	        buf.cmd = cmd;
	        buf.key = key;
	        add_queue(queue_list, buf);
	    }
	}
	int num_blocks = 1;
	int num_th_per_blk = queue_list->tail;
	char * pre_cmd_h;
	u_int16_t *pre_key_h, *aft_key_h, *alpha;
	size_t memSize_cmd = num_blocks*num_th_per_blk*sizeof(char);
	size_t memSize_key = num_blocks*num_th_per_blk*sizeof(u_int16_t);
	pre_cmd_h = (char*) malloc(memSize_cmd);
	pre_key_h = (u_int16_t*) malloc(memSize_key);
	aft_key_h = (u_int16_t*)malloc(memSize_key);
	alpha= (u_int16_t*)malloc(memSize_key);
	// save all keys and cmds to arrays
	for (int i = 0; i < num_th_per_blk; i++) {
	    pre_cmd_h[i] = queue_list->work_queue[i].cmd;
	    pre_key_h[i] = queue_list->work_queue[i].key;
	}
	free(queue_list);
	printf ("finish reading!\n");
	//create four “managing threads” to manage remote MPI processes.   
    #pragma omp parallel for num_threads(NUM_THREAD)
    for (int i = 0; i < num_th_per_blk; i++) {
        int id = omp_get_thread_num()+1;
        char cmd = pre_cmd_h[i];
        MPI_Status status1, status2, status3, status4;
	MPI_Status status5, status6, status7, status8;
        u_int16_t key = pre_key_h[i];
        u_int16_t a,res;
	c_t = clock();
// send each work unit to its coresponding id remote MPI process
        MPI_Send(&cmd,1,MPI_CHAR,id,0,MPI_COMM_WORLD);
        MPI_Send(&key,1,MPI_INT,id,0,MPI_COMM_WORLD);
	MPI_Send(&a,1,MPI_INT,id,0,MPI_COMM_WORLD);
	MPI_Send(&res,1,MPI_INT,id,0,MPI_COMM_WORLD);
	MPI_Send(&time_c, 1, MPI_DOUBLE, id,0,MPI_COMM_WORLD);
	MPI_Send(&time_p,1,MPI_DOUBLE,id,0,MPI_COMM_WORLD);
	c_t = clock() - c_t;
	time_t += abs(c_t);
	c_t = clock();
//receive the results from the thread’s corresponding remote MPI process
        MPI_Recv(&key,1,MPI_INT,id,0,MPI_COMM_WORLD,&status2);
        MPI_Recv(&a,1,MPI_INT,id,0,MPI_COMM_WORLD,&status3);
        MPI_Recv(&res,1,MPI_INT,id,0,MPI_COMM_WORLD,&status4);
	MPI_Recv(&time_c,1,MPI_DOUBLE,id,0,MPI_COMM_WORLD,&status5);
	MPI_Recv(&time_p,1,MPI_DOUBLE,id,0,MPI_COMM_WORLD,&status6);
	c_t = clock() - c_t;
	time_t += abs(c_t);
        alpha[i] = a;
        aft_key_h[i] = res;
	iter++;
	//print work results from a buffer of completed tasks 
	printf("%d cmd:%c key:%d alpha:%d res:%d\n",iter, pre_cmd_h[i],pre_key_h[i],alpha[i],aft_key_h[i]);
// free memory and finalize mpi when work is done
	if (iter == num_th_per_blk+1) {
		printf("%d %d\n",iter,num_th_per_blk);
		flag = 1;
		free(pre_cmd_h);
		free(pre_key_h);
		free(aft_key_h);
		free(alpha);
	    	MPI_Finalize();
				}
	}
    }
// remote MPI process begin work
if (rank != 0) {
	while(flag != 1) {
        char cmd;
        u_int16_t key, alpha, res;
	c_t = clock();
        MPI_Status status1, status2, status3, status4,status10;
	MPI_Status status5, status6, status7, status8;
//receive work units, one at a time, from a corresponding managing thread
        MPI_Recv(&cmd,1,MPI_CHAR,0,0,MPI_COMM_WORLD,&status1);
        MPI_Recv(&key,1,MPI_INT,0,0,MPI_COMM_WORLD, &status2);
	MPI_Recv(&alpha,1,MPI_INT,0,0,MPI_COMM_WORLD, &status3);
	MPI_Recv(&res,1,MPI_INT,0,0,MPI_COMM_WORLD, &status4);
	MPI_Recv(&time_c,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status5);
	MPI_Recv(&time_p,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status6);
	c_t = clock() - c_t;
	time_t += abs(c_t);
                if (cmd == 'A') {
		    c_p = clock();
                    alpha = transformA(key);
		    c_p = clock() - c_p;
		    time_p += abs(c_p);
		    c_c = clock();
                    res = transformA((alpha + 1) % 1000);
		    c_c = clock() - c_c;
		    time_c += abs(c_c);
                }
                if (cmd== 'B') {
		    c_p = clock();
                    alpha = transformB(key);
		    c_p = clock() - c_p;
		    time_p += abs(c_p);
		    c_c = clock();
                    res = transformB((alpha + 1) % 1000);
		    c_c = clock() - c_c;
		    time_c += abs(c_c);
                }
                if (cmd == 'C') {
		    c_p = clock();
                    alpha = transformC(key);
		    c_p = clock() - c_p;
		    time_p += abs(c_p);
		    c_c = clock();
                    res = transformC((alpha + 1) % 1000);
		    c_c = clock() - c_c;
		    time_c += abs(c_c);
                }
                if (cmd == 'D') {
		    c_p = clock();
                    alpha = transformD(key);
		    c_p = clock() - c_p;
		    time_p += abs(c_p);
		    c_c = clock();
                    res = transformD((alpha + 1) % 1000);
		    c_c = clock() - c_c;
		    time_c += abs(c_c);
                }
	c_t = clock();
//send the requested transform, initial input value, intermediate value α and final result value back to the corre- sponding managing thread
        MPI_Send(&key,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(&alpha,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(&res,1,MPI_INT,0,0,MPI_COMM_WORLD);
	MPI_Send(&time_c, 1, MPI_DOUBLE,0,0,MPI_COMM_WORLD);
	MPI_Send(&time_p,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
	c_t = clock() - c_t;
	time_t += abs(c_t);
	}
    }
   	printf("the producer time is %lf seconds\n", time_p/CLOCKS_PER_SEC);
	printf("the consumer time is %lf seconds\n", time_c/CLOCKS_PER_SEC);
	printf("message trasformation time is %lf seconds\n", time_t/CLOCKS_PER_SEC);
        return 0;
}
