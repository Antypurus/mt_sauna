#include "utils.h"

#include <time.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

double GetTimeSinceProgramStartup(double startup_time)
{
	struct timespec spec;
	if (clock_gettime(CLOCK_MONOTONIC, &spec) == -1)
	{
		printf("Error retrieving system time.\n");
		exit(1);
	}
	time_t time_in_s = spec.tv_sec;
	long time_in_ns = spec.tv_nsec;
	unsigned long long int full_time_in_ns = time_in_s * 10e9 + time_in_ns;
	double curr_time = round((double)full_time_in_ns / 10e4);
	curr_time /= 10e2; // milliseconds with 2 decimal cases

	return curr_time - startup_time;
}