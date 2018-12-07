 /*
 Author: Deren Kong (Kong.325)
 CSE5441  lab3
 Description: a simple openmp program to accomplish producer and consumer function
 Submitted on : 11/13/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <omp.h> 
#include "pc3.h"
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
void producer(queue *queue_list, int *run_time, double *run_time2) {
	char buffer[20];
	char temp_key[N];
	char cmd = ' ';
	//read each line to buffer
	while(fgets(buffer, 20, stdin) != NULL) {
	sscanf(buffer, "%c  %s", &cmd, temp_key);
	u_int16_t key = (u_int16_t)(atoi(temp_key));
	work_entry buf;
	clock_t t;
	t = clock();
	time_t timer;
	timer = time(NULL);
    if (cmd == 'X') {
     		buf.cmd = 'X';
		add_queue(queue_list,buf);
		t = clock() - t;
		*(run_time) += t;
		consumer(queue_list, run_time, run_time2);
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
        #pragma omp critical
	{
	time_t cur = time(NULL);
	*run_time2 += difftime(cur,timer);
	t = clock() - t;
	*run_time += t;
	}
	if (queue_list->full) {
		consumer(queue_list, run_time, run_time2);
	}
	}
	}

// consumer are able to transform the encoded key 
// and print result with stdout
void consumer(queue *queue_list, int * run_time, double *run_time2) {
	while (true){
	work_entry out;
        del_queue(queue_list, &out);
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
		return;	
	}
	printf("Q:%d	%c	%hd	%hd\n", queue_list->head, out.cmd, out.key, de_v);
	#pragma omp critical
	{
	time_t cur = time(NULL);
	*(run_time2) += difftime(cur,timer);
	t = clock() - t;
	*(run_time) += t;
	}
	if (queue_list->empty) {
		producer(queue_list, run_time, run_time2);
		return;
	}
	}
	}

int main(int argc, char* argv[]) {
	int nt = atoi(argv[1]);
	queue * queue_list = init_queue();
        if (queue_list == NULL) {
        	printf("Failed to init!\n");
        }
	bool flag = false;
	int run_time = 0;
	double run_time2 = 0.0;
	#pragma omp parallel num_threads(nt)  
	{
		printf("thread numbers is %d\n", omp_get_thread_num());
		producer(queue_list, &run_time, &run_time2);
	}
	printf("The total runtime of producer and consumer is %d clicks (%f seconds).\n", (run_time/nt),((float )run_time)/(nt*CLOCKS_PER_SEC));
	printf("The total runtime2 is %lf seconds.\n", (run_time2)/nt);
	destroy_queue(queue_list);
	return 0;
}



