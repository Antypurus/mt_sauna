#ifndef __VECTOR_INT
#define __VECTOR_INT

#include <stdio.h>
#include <pthread.h>

typedef struct
{
	size_t size;
	size_t capacity;
	pthread_t *data;
} vector_tid;

vector_tid *new_vector_tid();
void push_back_tid(vector_tid *vec, pthread_t tid);
pthread_t get_tid_at(const vector_tid * vec, size_t idx);
void set_tid_at(vector_tid *vec, size_t idx, pthread_t tid);

#endif
