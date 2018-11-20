 /*
 Author: Deren Kong (Kong.325)
 CSE5441  lab1
 Description: a simple program to accomplish producer and consumer function
 Compilation: using the commend line:
	$icc pc_lab_update.c transform.o 
	$./a.out < data.dms > result.txt
 Submitted on : 09/26/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include "pc_lab_update.h"
#define UINT_MAX 65535 
#define N 5

// producer are able to transform each key and fill
//that into work queue, when the queue has been filled, it will stop and wait for 
//consumer to clean it. 
// saved data in the work queue
void producer(int *total_runtime, work_entry work_queue[], int *start) {
	clock_t t;
        t = clock();
	// if the last element in work queue is not empty
	// it means the queue has been filled and should return
	char buffer[20];
	char temp_key[N];
	char cmd = ' ';
	work_entry buf;
	// temp_count is used to record the current position of the element in the working queue
	int temp_count = *start-N;
	//read each line to buffer
	while(fgets(buffer, 20, stdin) != NULL) {
	sscanf(buffer, "%c  %s", &cmd, temp_key);
	// if cmd is X, then record its position and quit
	if (cmd == 'X') {
		work_queue[temp_count].cmd = 'X';
		*start = temp_count;
		t = clock() - t;
		*total_runtime += t;
		return;
	}
	u_int16_t key = UINT_MAX;
	key = (u_int16_t)(atoi(temp_key));
	// create a work buffer entry containing the current
	//  value of cmd along with it’s encoded key.
	if (cmd == 'A' && key >= 0 && key <= 1000) {
		work_queue[temp_count].cmd = cmd;
		work_queue[temp_count].key = transformA(key);
		temp_count++;
	}
	if (cmd == 'B' && key >= 0 && key <= 1000) {
		work_queue[temp_count].cmd = cmd;
		work_queue[temp_count].key = transformB(key);
		temp_count++;
	}
	if (cmd == 'C' && key >= 0 && key <= 1000) {
		work_queue[temp_count].cmd = cmd;
		work_queue[temp_count].key = transformC(key);
		temp_count++;
	}
	if (cmd == 'D' && key >= 0 && key <= 1000) {
		work_queue[temp_count].cmd = cmd;
		work_queue[temp_count].key = transformD(key);
		temp_count++;
	}
	//if position become N after addition that mean the queue has been filled and producer has to quit
	if (temp_count == N) {
		t = clock() - t;
		*total_runtime += t;
		return;
	}
	}
 }

// consumer are able to transform the encoded key 
// and print result with stdout
void consumer(work_entry work_queue[], int *total_runtime, int *count) {
	clock_t t;
	t = clock();
	u_int16_t de_v = UINT_MAX;
	// temp_count record current element's position
	int temp_count = 0;
	while (temp_count < *count){
	if (work_queue[temp_count].cmd == 'A') {
		de_v = transformA(work_queue[temp_count].key);
	}
	if (work_queue[temp_count].cmd == 'B') {
		de_v = transformB(work_queue[temp_count].key);
	}
	if (work_queue[temp_count].cmd == 'C') {
		de_v = transformC(work_queue[temp_count].key);
	}
	if (work_queue[temp_count].cmd == 'D') {
		de_v = transformD(work_queue[temp_count].key);
	}
	printf("Q:%d	%c	%hd	%hd\n", temp_count, work_queue[temp_count].cmd, work_queue[temp_count].key, de_v);
	//after consume each time, the element should be deleted
	work_queue[temp_count].cmd = ' ';
	work_queue[temp_count].key = UINT_MAX;
	temp_count++;
	}
	t = clock() - t;
	*total_runtime += t;
}

int main(int argc, char *argv[]) {
	int total_runtime = 0;
	int queue_size = N;
	printf ("begin");
	// initialize the work_queue with ' ' for cmd and UINT_MAX for key
	work_entry * work_queue = (work_entry *) malloc(sizeof(work_entry) * queue_size);
	for (int i = 0; i < N; i++) {
		work_queue[i].cmd = ' ';
		work_queue[i].key = UINT_MAX;
	}
	//(count-1) record the end index, it can take an different value when
	//the queue isn't filled but has to quit due to meet with 'X'
	int count = N;
	while (true) {
		producer(&total_runtime, work_queue, &count);
		consumer(work_queue, &total_runtime, &count);
		//if count is not same as N that means meet 'X'
		if (count != N) {
			break;
		}
	}
	free(work_queue);
	printf("The total runtime of producer and consumer is %d clicks (%f seconds).\n", total_runtime,((float)total_runtime)/CLOCKS_PER_SEC);
	return 0;
}



