/*
 *  Author: Deren Kong (Kong.325)
 *  CSE5441  lab1
 *  Description: a simple program to accomplish producer and consumer function
 *  Compilation: using the commend line:
 *          $icc pc_lab.c transform.o 
 *          $./a.out < data.dms > result.txt
 *  Submitted on : 09/26/2018
 */
#ifndef __PC_LAB_UPDATE__
#define __PC_LAB_UPDATE__
typedef struct 
{
	char cmd;
	u_int16_t key; 
} work_entry;
u_int16_t transformA(u_int16_t input_val);
u_int16_t transformB(u_int16_t input_val);
u_int16_t transformC(u_int16_t input_val);
u_int16_t transformD(u_int16_t input_val);
void producer(int *total_runtime, work_entry work_queue[], int *start);
void consumer(work_entry work_queue[], int *total_runtime, int *count);

#endif

