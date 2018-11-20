 /*
 Author: Deren Kong (Kong.325)
 CSE5441  lab2
 Description: a simple pthread program to accomplish producer and consumer function
 Submitted on : 11/05/2018
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include "pc2.h"
#define UINT_MAX 65535 

queue *init_queue() {	
	queue * queue_list;
	queue_list = (queue*) malloc (sizeof(queue));
	int queue_size = N;
	// initialize the work_queue with ' ' for cmd and UINT_MAX for key
	for (int i = 0; i < N; i++) {
		queue_list->work_queue[i].cmd = ' ';
		queue_list->work_queue[i].key = UINT_MAX;
	}
	queue_list->empty = true;
	queue_list->full = false;
	queue_list->head = 0;
	queue_list->tail = 0;
	queue_list->mut = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	queue_list->condp = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
	queue_list->condc = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(queue_list->mut, NULL);
	pthread_cond_init(queue_list->condp, NULL);
	pthread_cond_init(queue_list->condc, NULL);
	double total_runtime2 = 0.0;
	int total_runtime = 0;
	queue_list->total_runtime2 = &total_runtime2;
	queue_list->total_runtime = &total_runtime;
	return queue_list;
}

void destroy_queue(queue *q) {
 	pthread_mutex_destroy(q->mut);
	pthread_cond_destroy(q->condp);
	pthread_cond_destroy(q->condc);
	free(q->mut);
	free(q->condp);
	free(q->condc);
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
void *producer(void *q) {
	queue * queue_list;
	queue_list = (queue*) q;
	char buffer[20];
	char temp_key[N];
	char cmd = ' ';
	//read each line to buffer
	while(fgets(buffer, 20, stdin) != NULL) {
	sscanf(buffer, "%c  %s", &cmd, temp_key);
	pthread_mutex_lock(queue_list->mut);
	//if queue is full, then wait...
	while(queue_list->full) {
		pthread_cond_wait(queue_list->condp, queue_list->mut);
	}
	u_int16_t key = (u_int16_t)(atoi(temp_key));
	//using mutex and cond to implement the thread process for producer
	pthread_mutex_unlock(queue_list->mut);
	pthread_cond_signal(queue_list->condc);
	work_entry buf;
	clock_t t;
	t = clock();
	time_t timer;
	timer = time(NULL);
       	if (cmd == 'X') {
     		buf.cmd = 'X';
		add_queue(queue_list,buf);
		t = clock() - t;
		*(queue_list->total_runtime) += t;
		return (NULL);
	}
	if (cmd == 'A' && key >= 0 && key <= 1000) {
		buf.cmd = cmd;
		buf.key = transformA(key);
		add_queue(queue_list, buf);
	}
	if (cmd == 'B' && key >= 0 && key <= 1000) {
		buf.cmd = cmd;
		buf.key = transformB(key);
		add_queue(queue_list, buf);
	}
	if (cmd == 'C' && key >= 0 && key <= 1000) {
		buf.cmd = cmd;
		buf.key = transformC(key);
		add_queue(queue_list, buf);
	}
	if (cmd == 'D' && key >= 0 && key <= 1000) {
		buf.cmd = cmd;
		buf.key = transformD(key);
		add_queue(queue_list, buf);
	}
	time_t cur = time(NULL);
	*(queue_list->total_runtime2) += difftime(cur,timer);
	t = clock() - t;
	*(queue_list->total_runtime) += t;
	}
	pthread_exit((void*)0);
	}

// consumer are able to transform the encoded key 
// and print result with stdout
void *consumer(void * q) {
	queue * queue_list;
	queue_list = (queue*)q;
	while (true){
	pthread_mutex_lock(queue_list->mut);
	//if queue is empty, then wait...
	while (queue_list->empty){
		pthread_cond_wait(queue_list->condc, queue_list->mut);
	}
	work_entry out;
        del_queue(queue_list, &out);
	//using mutex and cond to implement thread for consumer
	pthread_mutex_unlock(queue_list->mut);
	pthread_cond_signal(queue_list->condp);
	u_int16_t de_v;
	clock_t t;
	t = clock();
	time_t timer;
	timer = time(NULL);
	if (out.cmd == 'A') {
		de_v = transformA(out.key);
	}
	if (out.cmd == 'B') {
		de_v = transformB(out.key);
	}
	if (out.cmd == 'C') {
		de_v = transformC(out.key);
	}
	if (out.cmd == 'D') {
		de_v = transformD(out.key);
	}
	if (out.cmd == 'X') {
		break;	
	}
	printf("Q:%d	%c	%hd	%hd\n", queue_list->head, out.cmd, out.key, de_v);
	time_t cur = time(NULL);
	*(queue_list->total_runtime2) += difftime(cur,timer);
	t = clock() - t;
	*(queue_list->total_runtime) += t;
	}
      	pthread_exit((void*)0);
	}

int main(int argc, char *argv[]) {
	queue * queue_list = init_queue();
        if (queue_list == NULL) {
        	printf("Failed to init!\n");
        }
	pthread_t threadp, threadc;
	time_t timer = time(NULL);
	pthread_create(&threadp, NULL, producer, (void*)queue_list);
	pthread_create(&threadc, NULL, consumer, (void*)queue_list);
	timer = time(NULL) - timer;
	pthread_join(threadp, NULL);
	pthread_join(threadc, NULL);
	printf("The total runtime of producer and consumer is %d clicks (%f seconds).\n", *(queue_list->total_runtime)/2,((float )*(queue_list->total_runtime))/(2*CLOCKS_PER_SEC));
	printf("The total runtime2 is %lf seconds.\n", *(queue_list->total_runtime2)/2);
	destroy_queue(queue_list);
	return 0;
}



