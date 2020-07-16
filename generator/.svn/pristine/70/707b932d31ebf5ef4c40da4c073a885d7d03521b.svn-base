#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <pthread.h>

#include "utils.h"

#define MALE 'M'
#define FEMALE 'F'
#define ACCEPTED 1
#define REJECTED 0
#define LOG_FILENAME "/tmp/ger."
#define GERADOR_LOG_MESSAGE_LENGTH 255


unsigned int RETRY_INTERVAL; //in micro-seconds
unsigned int FIRSTS_REQUESTS_SENT = 0;


typedef struct
{
	unsigned int serial_number;
	char gender;
	unsigned int duration;
	char status;
} Request;

typedef struct
{
	unsigned int serial_number;
	unsigned int times_rejected;
} StatusPair;

typedef struct
{
	unsigned int male_generated_requests;
	unsigned int female_generated_requests;
	unsigned int male_rejected_requests;
	unsigned int female_rejected_requests;
	unsigned int male_discarded_requests;
	unsigned int female_discarded_requests;
} Statistics;

typedef struct
{
	double init_time;
	unsigned int num_requests;
	int fifo_output;
	int fifo_input;
	int log_filedes;
	StatusPair **status_arr;
	Statistics *stats;
} ParametersStruct;



Request *GenerateRequest(unsigned int max_duration)
{
	static unsigned int curr_serial_number = 1;

	Request *request = malloc(sizeof(Request));
	if (request == NULL)
	{
		perror("Allocating memory");
		exit(1);
	}
	request->serial_number = curr_serial_number;
	curr_serial_number++;

	unsigned int generated_num = rand() % 2;
	if (generated_num == 0)
		request->gender = MALE;
	else
		request->gender = FEMALE;

	request->duration = (rand() % max_duration) + 1;

	return request;
}

void OpenFifos(ParametersStruct *parameters)
{
	if ((parameters->fifo_output = open("/tmp/entrada", O_WRONLY)) == -1)
	{
		perror("Openning fifo entrada");
		exit(1);
	}
	//although the file name is "rejeitados" the requests that are saved onto it are not only the rejected ones but also the accepted (as an improvement). The file name was kept this to fallow the specification
	if ((parameters->fifo_input = open("/tmp/rejeitados", O_RDONLY)) == -1)
	{
		perror("Openning fifo rejeitados");
		exit(1);
	}
}

int InitLogFile()
{
	char log_filename[50] = LOG_FILENAME;
	if (sprintf(log_filename + strlen(log_filename), "%d", getpid()) < 0)
	{
		printf("Error creating gerador log filename\n");
		exit(1);
	}
	int log_filedes;
	if ((log_filedes = open(log_filename, O_CREAT | O_WRONLY | O_EXCL, 0644)) == -1)
	{
		perror("Opening gerador log file");
		exit(1);
	}


	char log_file_first_line[GERADOR_LOG_MESSAGE_LENGTH];
	if (sprintf(log_file_first_line, "%-15s - %-9s - %-9s: %-5s - %-9s - %-9s\n", "inst", "pid", "p", "g", "dur", "tip") < 0)
	{
		printf("Error creating log first line to gerador log\n");
		exit(1);
	}
	if (write(log_filedes, log_file_first_line, strlen(log_file_first_line)) != strlen(log_file_first_line))
	{
		perror("Writing first line to gerador log file");
		exit(1);
	}

	return log_filedes;
}


void WriteToLogFile(const Request *request, double inst, int filedes, pid_t pid, char *tip)
{
	char message[GERADOR_LOG_MESSAGE_LENGTH];
	if (sprintf(message, "%-15.2f - %-9d - %-9d: %-5c - %-9d - %-9s\n", inst, pid, request->serial_number, request->gender, request->duration, tip) < 0)
	{
		printf("Error creating gerador log message\n");
		exit(1);
	}

	if (write(filedes, message, strlen(message)) != strlen(message))
	{
		perror("Writing line to gerador log file");
		exit(1);
	}
}


void IncrementGeneratedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
	{
		stats->male_generated_requests++;
	}
	else
	{
		stats->female_generated_requests++;
	}
}


void IncrementRejectedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
	{
		stats->male_rejected_requests++;
	}
	else
		stats->female_rejected_requests++;
}


void IncrementDiscardedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
		stats->male_discarded_requests++;
	else
		stats->female_discarded_requests++;
}


StatusPair* RequestToStatusPair(const Request *request, const ParametersStruct *parameters)
{
	for (int i = 0; i < parameters->num_requests; i++)
	{
		if (parameters->status_arr[i]->serial_number == request->serial_number)
			return parameters->status_arr[i];
	}

	return NULL;
}


unsigned int IsToDiscardRequest(const StatusPair *status)
{
	if (status->times_rejected == 3)
		return 1;

	return 0;
}

void IncrementTimesRejected(StatusPair *status)
{
	status->times_rejected++;
}

void HandleReponseFromSauna(const Request *request, ParametersStruct *parameters, double read_inst, unsigned int *num_requests_processed)
{
	if (request->status == ACCEPTED)
	{
		(*num_requests_processed)++;
		return;
	}

	//Request REJECTED
	pid_t pid = getpid();
	while (FIRSTS_REQUESTS_SENT == 0)
		usleep(2000); //2ms

	IncrementRejectedRequests(parameters->stats, request->gender);
	WriteToLogFile(request, read_inst, parameters->log_filedes, pid, "REJEITADO");

	StatusPair *status;
	if ((status = RequestToStatusPair(request, parameters)) == NULL)
	{
		printf("Internal error occurred: Request undefined\n");
		exit(1);
	}
	IncrementTimesRejected(status);

	if (IsToDiscardRequest(status))
	{
		(*num_requests_processed)++;
		IncrementDiscardedRequests(parameters->stats, request->gender);
		double inst = GetTimeSinceProgramStartup(parameters->init_time);
		WriteToLogFile(request, inst, parameters->log_filedes, pid, "DESCARTADO");
	}
	else
	{
		usleep(RETRY_INTERVAL);
		if (write(parameters->fifo_output, request, sizeof(Request)) != sizeof(Request))
		{
			perror("Writing request to fifo entradas");
			exit(1);
		}
		double inst = GetTimeSinceProgramStartup(parameters->init_time);
		WriteToLogFile(request, inst, parameters->log_filedes, pid, "PEDIDO");
	}
}

void *ListenResponseFromSauna(void *arg)
{
	static unsigned int num_requests_processed = 0;

	Request *request = malloc(sizeof(Request));
	if (request == NULL)
	{
		perror("Allocating memory");
		exit(1);
	}
	ParametersStruct *parameters = (ParametersStruct *)arg;

	unsigned int request_size = sizeof(Request);
	while (num_requests_processed != parameters->num_requests)
	{
		double inst = GetTimeSinceProgramStartup(parameters->init_time);
		if ((read(parameters->fifo_input, request, request_size)) != request_size)
		{
			printf("Error: Reading request status from fifo\n");
			exit(1);
		}

		HandleReponseFromSauna(request, parameters, inst, &num_requests_processed);
	}
	return NULL;
}


void InitializeStats(Statistics *stats)
{
	stats->male_generated_requests = 0;
	stats->female_generated_requests = 0;
	stats->male_rejected_requests = 0;
	stats->female_rejected_requests = 0;
	stats->male_discarded_requests = 0;
	stats->female_discarded_requests = 0;
}


void PrintStatistics(const Statistics *stats)
{
	unsigned int total_generated_requests = stats->male_generated_requests + stats->female_generated_requests;
	printf("\nTotal number of generated requests: %d\n", total_generated_requests);
	printf("Male number of generated requests: %d\n", stats->male_generated_requests);
	printf("Female number of generated requests: %d\n", stats->female_generated_requests);

	unsigned int total_rejected_requests = stats->male_rejected_requests + stats->female_rejected_requests;
	printf("\nTotal number of request rejections: %d\n", total_rejected_requests);
	printf("Male number of request rejections: %d\n", stats->male_rejected_requests);
	printf("Female number of request rejections: %d\n", stats->female_rejected_requests);

	unsigned int total_discarded_requests = stats->male_discarded_requests + stats->female_discarded_requests;
	printf("\nTotal number of discarded rejections: %d\n", total_discarded_requests);
	printf("Male number of discarded rejections: %d\n", stats->male_discarded_requests);
	printf("Female number of discarded rejections: %d\n", stats->female_discarded_requests);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Incorrect number of arguments\n");
		exit(1);
	}

	ParametersStruct parameters;
	parameters.init_time = GetTimeSinceProgramStartup(0.0);
	parameters.num_requests = atoi(argv[1]);
	unsigned int max_duration = atoi(argv[2]);
	RETRY_INTERVAL = max_duration / 2;
	RETRY_INTERVAL *= 1000; //convert to micro-seconds
	OpenFifos(&parameters);
	parameters.log_filedes = InitLogFile();
	

	Statistics stats;
	InitializeStats(&stats);

	parameters.stats = &stats;
	StatusPair* status_arr[parameters.num_requests];
	parameters.status_arr = status_arr;
	pthread_t tid;
	if (pthread_create(&tid, NULL, ListenResponseFromSauna, &parameters) != 0)
	{
		printf("Error creating thread to listen responses from sauna\n");
		exit(1);
	}

	srand(time(NULL));
	pid_t pid = getpid();
	for (int i = 0; i < parameters.num_requests; i++)
	{
		Request *request = GenerateRequest(max_duration);
		IncrementGeneratedRequests(&stats, request->gender);
		double inst = GetTimeSinceProgramStartup(parameters.init_time);
		if (write(parameters.fifo_output, request, sizeof(Request)) != sizeof(Request))
		{
			perror("Writing to fifo entradas");
			exit(1);
		}
		WriteToLogFile(request, inst, parameters.log_filedes, pid, "PEDIDO");
		StatusPair *status = malloc(sizeof(StatusPair));
		if (status == NULL)
		{
			perror("Allocating memory");
			exit(1);
		}
		status->times_rejected = 0;
		status->serial_number = request->serial_number;
		status_arr[i] = status;
		free(request);
	}

	//sincronization to garantee that the generated requests are send all by the first time before a second attemp of one of them happens
	FIRSTS_REQUESTS_SENT = 1;

	if (pthread_join(tid, NULL) != 0)
	{
		printf("Error joining thread to exit\n");
		exit(1);
	}

	PrintStatistics(&stats);

	//Closes fifos and file before ending the program
	if (close(parameters.fifo_output) == -1)
	{
		perror("Closing fifo /tmp/entrada");
		exit(1);
	}
	if (close(parameters.fifo_input) == -1)
	{
		perror("Closing fifo /tmp/rejeitados");
		exit(1);
	}
	if (close(parameters.log_filedes) == -1)
	{
		perror("Closing log file");
		exit(1);
	}

	//Deallocating the dynamic memory of the requests
	for (int i = 0; i < parameters.num_requests; i++)
		free(status_arr[i]);

	return 0;
}