 /*
 Author: Deren Kong (Kong.325)
 CSE5441  lab4
 Description: a simple openmp program to accomplish producer and consumer function
 compile: qsub -I -l walltime=00:59:00 -l nodes=1:gpus=1,mem=4gb -A PAS1421
	  sh ./fire_cuda_v2.sh
 Submitted on : 11/27/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "pc4_v2.h"
#define EMPTY 9999 
__device__ u_int16_t transformA(u_int16_t input_val);
__device__ u_int16_t transformB(u_int16_t input_val);
__device__ u_int16_t transformC(u_int16_t input_val);
__device__ u_int16_t transformD(u_int16_t input_val);
__global__ void transform(char* cmd, u_int16_t *pre, u_int16_t *aft) {
	int id = threadIdx.x + blockIdx.x * blockDim.x;
	//for (int i = 0; i < 10; i++) {
	if (cmd[id] == 'A') {
		aft[id] = transformA(pre[id]);
	}
	if (cmd[id] == 'B') {
		aft[id] = transformB(pre[id]);
	}
	if (cmd[id] == 'C') {
		aft[id] = transformC(pre[id]);
	}
	if (cmd[id] == 'D') {
		aft[id] = transformD(pre[id]);
	}
}
__global__ void transform_print(char* cmd, u_int16_t *pre, u_int16_t *aft) {
	int id = threadIdx.x + blockIdx.x * blockDim.x;
	if (cmd[id] == 'A') {
		aft[id] = transformA(pre[id]);
	}
	if (cmd[id] == 'B') {
		aft[id] = transformB(pre[id]);
	}
	if (cmd[id] == 'C') {
		aft[id] = transformC(pre[id]);
	}
	if (cmd[id] == 'D') {
		aft[id] = transformD(pre[id]);
	}

	printf("Q:%d %c %hd %hd\n",id, cmd[id], pre[id], aft[id]);
}
queue *init_queue() {	
	queue * queue_list;
	queue_list = (queue*) malloc (sizeof(queue));
	for (int i = 0; i < N; i++) {
		queue_list->work_queue[i].cmd = ' ';
		queue_list->work_queue[i].key = EMPTY;
	}
	queue_list->empty = true;
	queue_list->full = false;
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
		q->full = true;
	}	
	q->empty = false;
}

void del_queue(queue *q, work_entry *out) {
	*out = q->work_queue[q->head];
	q->head++;
	if (q->head == N) {
		q->head = 0;
	}
	if (q->head == q->tail) {
		q->empty = true;	
	}
	q->full = false;
}

// producer are able to transform each key and fill
//that into work queue, when the queue has been filled, it will stop and wait for 
//consumer to clean it. 
void producer(queue *queue_list_a, queue* queue_list_b, queue* queue_list_c,
		queue* queue_list_d, int *run_time, double *run_time2) {
	char buffer[20];
	char temp_key[10];
	char cmd = ' ';
	//read each line to buffer
	while(fgets(buffer, 20, stdin) != NULL) {
	sscanf(buffer, "%c  %s", &cmd, temp_key);
	u_int16_t key = (u_int16_t)(atoi(temp_key));
	work_entry buf;
// this version use 4 queue_list to collect A B C D seperately 
	if (cmd == 'X') {
    		break;
	}
	if (cmd == 'A' && key <= 1000) {
		buf.cmd = cmd;
		buf.key = key;
		add_queue(queue_list_a, buf);
	}
	if (cmd == 'B' && key <= 1000) {
		buf.cmd = cmd;
		buf.key = key;
		add_queue(queue_list_b, buf);
	}
	if (cmd == 'C' && key <= 1000) {
		buf.cmd = cmd;
		buf.key = key;
		add_queue(queue_list_c, buf);
	}
	if (cmd == 'D' && key <= 1000) {
		buf.cmd = cmd;
		buf.key = key;
		add_queue(queue_list_d, buf);
	}
		}
	printf ("finish reading!\n");
	int num_blocks = 1;
	int num_th_per_blk_a = queue_list_a->tail;
	int num_th_per_blk_b = queue_list_b->tail;
	int num_th_per_blk_c = queue_list_c->tail;
	int num_th_per_blk_d = queue_list_d->tail;
	int num_th_per_blk = queue_list_a->tail + queue_list_b->tail + queue_list_c->tail + queue_list_d->tail;
	int old_num_th_per_blk = -1;
	if (N >= 2048) {
		num_blocks = 10;
		printf("original is %d ",num_th_per_blk);
		old_num_th_per_blk = num_th_per_blk;
		num_th_per_blk = ceil((double)num_th_per_blk / 10.0);
		printf("now is %d\n", num_th_per_blk);
	}
	char * pre_cmd_h, *pre_cmd_d;;
	u_int16_t *pre_key_h, *aft_key_h, *pre_key_d, *aft_key_d;	
	size_t memSize_cmd = num_blocks*num_th_per_blk*sizeof(char);
	size_t memSize_key = num_blocks*num_th_per_blk*sizeof(u_int16_t);
	pre_cmd_h = (char*) malloc(memSize_cmd);
	pre_key_h = (u_int16_t*) malloc(memSize_key);
	aft_key_h = (u_int16_t*)malloc(memSize_key);
	for (int i = 0; i < num_th_per_blk_a; i++) {
		pre_cmd_h[i] = queue_list_a->work_queue[i].cmd;
		pre_key_h[i] = queue_list_a->work_queue[i].key;
		//printf("%c %hd\n", pre_cmd_h[i], pre_key_h[i]);
	}
	for (int i = 0; i < num_th_per_blk_b; i++) {
		pre_cmd_h[num_th_per_blk_a + i] = queue_list_b->work_queue[i].cmd;
		pre_key_h[num_th_per_blk_a + i] = queue_list_b->work_queue[i].key;
		//printf("%c %hd\n", pre_cmd_h[num_th_per_blk_a+ i], pre_key_h[num_th_per_blk_a+ i]);
	}
	for (int i = 0; i <  num_th_per_blk_c; i++) {
		pre_cmd_h[num_th_per_blk_a + num_th_per_blk_b + i] = queue_list_c->work_queue[i].cmd;
		pre_key_h[num_th_per_blk_a + num_th_per_blk_b + i] = queue_list_c->work_queue[i].key;
		//printf("%c %hd\n", pre_cmd_h[num_th_per_blk_a + num_th_per_blk_b + i], pre_key_h[num_th_per_blk_a + num_th_per_blk_b + i]);
	}
	for (int i = 0; i < num_th_per_blk_d; i++) {
		pre_cmd_h[num_th_per_blk_a + num_th_per_blk_b + num_th_per_blk_c + i] = queue_list_d->work_queue[i].cmd;
		pre_key_h[num_th_per_blk_a + num_th_per_blk_b + num_th_per_blk_c + i] = queue_list_d->work_queue[i].key;
		//printf("%c %hd\n", pre_cmd_h[num_th_per_blk_a + num_th_per_blk_b + num_th_per_blk_c +i], pre_key_h[num_th_per_blk_a + num_th_per_blk_b + num_th_per_blk_c +i]);
	}
	clock_t t;
	t = clock();
	time_t timer;
	timer = time(NULL);

	cudaMalloc((void**)&pre_cmd_d, memSize_cmd);
	cudaMalloc((void**)&pre_key_d, memSize_key);
	cudaMalloc((void**)&aft_key_d, memSize_key);
	cudaMemcpy(pre_cmd_d, pre_cmd_h, memSize_cmd, cudaMemcpyHostToDevice);
	cudaMemcpy(pre_key_d, pre_key_h, memSize_key, cudaMemcpyHostToDevice);
	printf("launch kernel\n");
	dim3 dimGrid(num_blocks);
	dim3 dimBlock(num_th_per_blk);
	transform <<< dimGrid, dimBlock >>>(pre_cmd_d, pre_key_d, aft_key_d);

	printf("finish transform\n");
	cudaMemcpy(pre_cmd_h, pre_cmd_d, memSize_cmd, cudaMemcpyDeviceToHost);
	cudaMemcpy(pre_key_h, pre_key_d, memSize_key, cudaMemcpyDeviceToHost);

	cudaMemcpy(aft_key_h, aft_key_d, memSize_key, cudaMemcpyDeviceToHost);
	cudaFree(aft_key_d);
	cudaFree(pre_cmd_d);
	cudaFree(aft_key_d);
	printf ("finish producer!\n");	
	consumer(pre_cmd_h, aft_key_h, num_th_per_blk);
	time_t cur = time(NULL);
        *run_time2 += difftime(cur,timer);
        t = clock() - t;
        *run_time += t;
	}

// consumer are able to transform the encoded key 
// and print result with stdout
void consumer(char* cmd, u_int16_t* key, int size) {

	char* cmd_d;
	u_int16_t *key_d, *aft_key_h, *aft_key_d;
	int num_blocks = 1;
	int num_th_per_blk = size;
	if (N >= 2048) {
		num_blocks = 10;
	}
	size_t memSize_cmd = num_blocks*num_th_per_blk*sizeof(char);
	size_t memSize_key = num_blocks*num_th_per_blk*sizeof(u_int16_t);
	cudaMalloc((void**) &cmd_d, memSize_cmd);
	cudaMalloc((void**) &key_d, memSize_key);
	cudaMalloc((void**) &aft_key_d, memSize_key);
	cudaMemcpy(key_d, key, memSize_key, cudaMemcpyHostToDevice);
	cudaMemcpy(cmd_d, cmd, memSize_cmd, cudaMemcpyHostToDevice);

	dim3 dimGrid(num_blocks);
	dim3 dimBlock(num_th_per_blk);
	transform_print <<< dimGrid, dimBlock >>>(cmd_d, key_d, aft_key_d);

	printf("consumer cuda work done!\n");
	cudaMemcpy(aft_key_h, aft_key_d, memSize_key, cudaMemcpyDeviceToHost);
	cudaFree(cmd_d);
	cudaFree(key_d);
	cudaFree(aft_key_d);
	}

int main(int argc, char* argv[]) {
	queue* queue_list_a = init_queue();
	queue* queue_list_b = init_queue();
	queue* queue_list_c = init_queue();
	queue* queue_list_d = init_queue();
	int run_time = 0;
	double run_time2 = 0.0;
	producer(queue_list_a, queue_list_b, queue_list_c, queue_list_d, &run_time, &run_time2);
	printf("The total runtime of producer and consumer is %d clicks (%f seconds).\n", (run_time),((float )run_time)/(CLOCKS_PER_SEC));
	printf("The total runtime2 is %lf seconds.\n", (run_time2));
	free(queue_list_a);
	free(queue_list_b);
	free(queue_list_c);
	free(queue_list_d);
	return 0;
}



