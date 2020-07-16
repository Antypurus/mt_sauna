#include "vector_tid.h"

#include <stdlib.h> 

vector_tid *new_vector_tid()
{
	vector_tid *vec = malloc(sizeof(vector_tid));
	if (vec == NULL)
	{
		perror("Failed to allocate memory to vector");
		exit(1);
	}
	vec->size = 0;
	vec->capacity = 0;
	vec->data = NULL;

	return vec;
}

void push_back_tid(vector_tid *vec, pthread_t tid)
{
	if (vec->size == vec->capacity)
	{
		if (vec->capacity == 0) //base case, when the vector has no data
		{
			vec->data = malloc(1 * sizeof(pthread_t));
			if (vec->data == NULL)
			{
				perror("Failed to allocate memory to vector contents");
				exit(1);
			}
			vec->capacity++;
		}
		else
		{
			size_t new_capacity = vec->capacity * 1.5;
			if (new_capacity == vec->capacity)
				new_capacity++;
			vec->data = realloc(vec->data, new_capacity * sizeof(pthread_t));
			if (vec->data == NULL)
			{
				perror("Failed to reallocate memory to vector contents");
				exit(1);
			}
			vec->capacity = new_capacity;
		}
	}
	vec->data[vec->size] = tid;
	vec->size++;
}

pthread_t get_tid_at(const vector_tid * vec, size_t idx)
{
	if (idx >= vec->size)
	{
		printf("Vector subscript out of range!\n");
		exit(1);
	}
	return vec->data[idx];
}

void set_tid_at(vector_tid *vec, size_t idx, pthread_t tid)
{
	if (idx >= vec->size)
	{
		printf("Vector subscript out of range!\n");
		exit(1);
	}
	vec->data[idx] = tid;
}