/*
 * Author: Deren Kong
 * Description: simple hierarchical parallelism program using openmp and MVAPICH2
 * compile: make
 * submit date: 12/05/2018
*/
#ifndef __PC5__
#define __PC5__
#define N 5000
#include <stdio.h>
#include <stdlib.h>
typedef struct
{
    char cmd;
    u_int16_t key;
} work_entry;
typedef struct {
    work_entry work_queue[N];
    int empty, full;
    u_int16_t head, tail;
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

#endif
