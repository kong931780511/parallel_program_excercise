/*
 *  Author: Deren Kong (Kong.325)
 *  CSE5441  lab4
 *  Description: a simple cuda program to accomplish producer and consumer function
 *  Submitted on : 10/27/2018
 */
#ifndef __PC4_V2__
#define __PC4_V2__
#define N 10000 

typedef struct 
{
	char cmd;
	u_int16_t key; 
} work_entry;
typedef struct {
	work_entry work_queue[N];
	bool empty, full;
	u_int16_t head, tail;
} queue;
__device__ u_int16_t transformA(u_int16_t input_val);
__device__ u_int16_t transformB(u_int16_t input_val);
__device__ u_int16_t transformC(u_int16_t input_val);
__device__ u_int16_t transformD(u_int16_t input_val);
//initiate work_queue
queue *init_queue();
void destroy_queue(queue *q);
void add_queue(queue*q, work_entry element);
void del_queue(queue *q, work_entry *out);
void producer(queue *queue_list_a, queue* queue_list_b, queue* queue_list_c, queue* queue_list_d, int *run_time, double *run_time2);
void consumer(char* cmd, u_int16_t* key, int size);

#endif

