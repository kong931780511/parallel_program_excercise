/*
 *  Author: Deren Kong (Kong.325)
 *  CSE5441  lab2
 *  Description: a simple pthread program to accomplish producer and consumer function
 *  Submitted on : 10/05/2018
 */
#ifndef __PC2__
#define __PC2__
#define N 100 
#include <pthread.h>

typedef struct 
{
	char cmd;
	u_int16_t key; 
} work_entry;
// wrap the parameters for pthread 
typedef struct {
	work_entry work_queue[N];
	bool empty, full;
	u_int16_t head, tail;
	pthread_cond_t *condp, *condc;
	pthread_mutex_t *mut;
	//clock(3)
	int *total_runtime;
	//time(2)
	double *total_runtime2;
} queue;
u_int16_t transformA(u_int16_t input_val);
u_int16_t transformB(u_int16_t input_val);
u_int16_t transformC(u_int16_t input_val);
u_int16_t transformD(u_int16_t input_val);
//initiate work_queue
queue *init_queue();
void destroy_queue(queue *q);
void add_queue(queue*q, work_entry element);
void del_queue(queue *q, work_entry *out);
void *producer(void *q);
void *consumer(void *q);

#endif

