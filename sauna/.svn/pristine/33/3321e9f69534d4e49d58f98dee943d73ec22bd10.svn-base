#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <semaphore.h>
#include <pthread.h>

#include "vector_tid.h"
#include "utils.h"

#define MALE 'M'
#define FEMALE 'F'
#define ACCEPTED 1
#define REJECTED 0
#define SAUNA_LOG_MESSAGE_LENGTH 255
#define LOG_FILENAME "/tmp/bal."
#define mlock(a) pthread_mutex_lock(a)//macro for locking a mutex
#define munlock(a) pthread_mutex_unlock(a)//macro for unlocking a mutex

unsigned int MAX_LUGARES;
unsigned int LUGARES_LIVRES;
char GENDER = 'N';
pthread_mutex_t MUTEX_LUGARES = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_GENDER = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CONDITIONAL_VARIABLE = PTHREAD_COND_INITIALIZER;

typedef struct
{
	unsigned int serial_number;
	char gender;
	unsigned int duration;
	char status;
} Request;


typedef struct
{
	unsigned int male_received_requests;
	unsigned int female_received_requests;
	unsigned int male_rejected_requests;
	unsigned int female_rejected_requests;
	unsigned int male_served_requests;
	unsigned int female_served_requests;
} Statistics;


typedef struct
{
	double init_time;
	int fifo_output;
	int fifo_input;
	int log_filedes;
	Request *request;
	vector_tid *tids;
	Statistics *stats;
} ParametersStruct;
//this struct contains info needed by the various functions of the program, instantiated in the main, is passed by reference to the function.
//Instead of using global variables.

void MakeAndOpenFifos(ParametersStruct *parameters)
{
	if (mkfifo("/tmp/entrada", 0660) == -1)
	{
		printf("Error Creating Fifo de entrada\n");
		exit(1);
	}
	if (mkfifo("/tmp/rejeitados", 0660) == -1)
	{
		printf("Error Creating Fifo de rejeitados\n");
		exit(1);
	}
	if ((parameters->fifo_input = open("/tmp/entrada", O_RDONLY)) == -1)
	{
		perror("Openning fifo entrada");
		exit(1);
	}
	if ((parameters->fifo_output = open("/tmp/rejeitados", O_WRONLY)) == -1)
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
		printf("Error Creating log_filename String\n");
		exit(1);
	}
	int log_filedes;
	if ((log_filedes = open(log_filename, O_CREAT | O_WRONLY | O_EXCL, 0644)) == -1)
	{
		perror("Openning sauna log file");
		exit(1);
	}


	char log_file_first_line[SAUNA_LOG_MESSAGE_LENGTH];
	if (sprintf(log_file_first_line, "%-15s - %-9s - %-20s - %-9s: %-5s - %-9s - %-9s\n",
		"inst", "pid", "tid", "p", "g", "dur", "tip") < 0)
	{
		printf("Error Creating log_file_first_line String\n");
		exit(1);
	}
	if (write(log_filedes, log_file_first_line, strlen(log_file_first_line)) != strlen(log_file_first_line))
	{
		perror("Writing first line to sauna log file");
		exit(1);
	}

	return log_filedes;
}

void WriteToLogFile(const Request *request, double inst, int filedes, pid_t pid, pthread_t tid, const char *tip)
{
	char message[SAUNA_LOG_MESSAGE_LENGTH];
	if (sprintf(message, "%-15.2f - %-9d - %-20lu - %-9d: %-5c - %-9d - %-9s\n",
		inst, pid, tid, request->serial_number, request->gender, request->duration, tip) < 0)
	{
		printf("Error Creating message String\n");
		exit(1);
	}

	if (write(filedes, message, strlen(message)) != strlen(message))
	{
		perror("Writing line to sauna log file");
		exit(1);
	}
}


char GetCurrentGender()
{
	if (mlock(&MUTEX_GENDER) != 0) 
	{
		printf("Error Lockign Gender Mutex\n");
		exit(1);
	}
	return GENDER;
	//reader must unlock the mutex
}


void SetCurrentGender(char gender)
{
	if (mlock(&MUTEX_GENDER) != 0) 
	{
		printf("Error Lockign Gender Mutex\n");
		exit(1);
	}
	if (gender == MALE || gender == FEMALE || gender == 'N')
	{
		GENDER = gender;
	}
	if (munlock(&MUTEX_GENDER) != 0) 
	{
		printf("Error Unlockign Gender Mutex\n");
		exit(1);
	}
}


//returns the ammount of free space
unsigned int GetLugaresLivres()
{
	if (mlock(&MUTEX_LUGARES) != 0) 
	{
		printf("Error Locking Lugares Mutex\n");
		exit(1);
	}
	return LUGARES_LIVRES;
	//the one who reads needs to then unlock the mutex
}


void IncrementReceivedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
	{
		stats->male_received_requests++;
	}
	else
	{
		stats->female_received_requests++;
	}
}


void IncrementRejectedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
		stats->male_rejected_requests++;
	else
		stats->female_rejected_requests++;
}


void IncrementServedRequests(Statistics *stats, char gender)
{
	if (gender == MALE)
		stats->male_served_requests++;
	else
		stats->female_served_requests++;
}


int ShouldProcessRequest(const Request *request)
{
	char current_gender = GetCurrentGender();
	if (current_gender == 'N')
		return 1;

	return (request->gender == current_gender);
	//the one who reads needs to then unlock the mutex
}


void* ProcessRequest(void *arg)//to be launched in a thread
{
	ParametersStruct *parameters = (ParametersStruct*)arg;
	GENDER = parameters->request->gender;
	if (munlock(&MUTEX_GENDER) != 0)
	{
		printf("Error Unlocking Gender Mutex\n");
		exit(1);
	}

	if (mlock(&MUTEX_LUGARES) != 0) {
		printf("Error Locking Lugares Mutex\n");
		exit(1);
	}
	while (LUGARES_LIVRES < 1)
	{
		if (pthread_cond_wait(&CONDITIONAL_VARIABLE, &MUTEX_LUGARES) != 0)
		{
			printf("Error Waiting For Condition Variable\n");
			exit(1);
		}
	}
	LUGARES_LIVRES--;
	if (munlock(&MUTEX_LUGARES) != 0)
	{
		printf("Error Unlocking Lugares Mutex\n");
		exit(1);
	}

	int i = usleep(parameters->request->duration * 1000);
	while (i != 0)
	{
		i = usleep(i);
	}
	double inst = GetTimeSinceProgramStartup(parameters->init_time);

	if (mlock(&MUTEX_LUGARES) != 0)
	{
		printf("Error Locking Lugares Mutex\n");
		exit(1);
	}
	LUGARES_LIVRES++;
	if (LUGARES_LIVRES == MAX_LUGARES)
		SetCurrentGender('N');
	if (pthread_cond_signal(&CONDITIONAL_VARIABLE) != 0)
	{
		printf("Error Signaling Condition Varible\n");
		exit(1);
	}

	if (munlock(&MUTEX_LUGARES) != 0)
	{
		printf("Error Unlocking Lugares Mutex\n");
		exit(1);
	}


	pthread_t tid = pthread_self();
	WriteToLogFile(parameters->request, inst, parameters->log_filedes, getpid(), tid, "SERVIDO");
	free(parameters->request);
	free(parameters);

	return NULL;
}


void HandleRequest(ParametersStruct *parameters, Statistics *stats)
{
	if (!ShouldProcessRequest(parameters->request))
	{
		if (munlock(&MUTEX_GENDER) != 0)
		{
			printf("Error locking Gender Mutex\n");
			exit(1);
		}
		parameters->request->status = REJECTED;
		if (write(parameters->fifo_output, parameters->request, sizeof(Request)) == -1)
		{
			printf("Error Writhing To fifo output\n");
			exit(1);
		}
		double inst = GetTimeSinceProgramStartup(parameters->init_time);
		pthread_t tid = pthread_self();
		WriteToLogFile(parameters->request, inst, parameters->log_filedes, getpid(), tid, "REJEITADO");
		IncrementRejectedRequests(stats, parameters->request->gender);
		return;
	}

	parameters->request->status = ACCEPTED;

	//A copy of the parameters is needed because a copy of the Request is needed (attrib of the struct Parameters)
	//Since multiple threads will be reading in different times, the content of the memory pointed to by the arg cannot be changed before the thread reads it
	//(no memory leaks will ocurr since the thread function frees the buffer in the end)
	ParametersStruct* parameters_cpy = malloc(sizeof(ParametersStruct));
	memcpy(parameters_cpy, parameters, sizeof(ParametersStruct));
	//it is necessary to copy the attribute also, otherwise the request field of the new struct is the pointer (copied by last memcpy) which would result in the same problem
	Request *request_cpy = malloc(sizeof(Request));
	memcpy(request_cpy, parameters->request, sizeof(Request));
	parameters_cpy->request = request_cpy;

	if (write(parameters->fifo_output, parameters->request, sizeof(Request)) == -1)
	{
		printf("Error Writhing to fifo output\n");
		exit(1);
	}
	pthread_t tid;
	if (pthread_create(&tid, NULL, ProcessRequest, parameters_cpy) != 0)
	{
		printf("Error Creating ProcessRequest Thread\n");
		exit(1);
	}
	IncrementServedRequests(stats, parameters->request->gender);
	push_back_tid(parameters->tids, tid);
}


void InitializeStats(Statistics *stats)
{
	stats->male_received_requests = 0;
	stats->female_received_requests = 0;
	stats->male_rejected_requests = 0;
	stats->female_rejected_requests = 0;
	stats->male_served_requests = 0;
	stats->female_served_requests = 0;
}


void PrintStatistics(const Statistics *stats)
{
	unsigned int total_received_requests = stats->male_received_requests + stats->female_received_requests;
	printf("\nTotal number of received requests: %u\n", total_received_requests);
	printf("Male number of received requests: %u\n", stats->male_received_requests);
	printf("Female number of received requests: %u\n", stats->female_received_requests);

	unsigned int total_rejected_requests = stats->male_rejected_requests + stats->female_rejected_requests;
	printf("\nTotal number of rejected requests: %d\n", total_rejected_requests);
	printf("Male number of rejected requests: %d\n", stats->male_rejected_requests);
	printf("Female number of rejected requests: %d\n", stats->female_rejected_requests);

	unsigned int total_served_requests = stats->male_served_requests + stats->female_served_requests;
	printf("\nTotal number of served requests: %d\n", total_served_requests);
	printf("Male number of served requests: %d\n", stats->male_served_requests);
	printf("Female number of served requests: %d\n", stats->female_served_requests);
}

void CloseAndDeleteFifos(const ParametersStruct *parameters)
{
	if (close(parameters->fifo_output) == -1)
	{
		perror("Closing fifo /tmp/entrada");
		exit(1);
	}
	if (unlink("/tmp/entrada"))
	{
		perror("Closing fifo fifo /tmp/entrada");
		exit(1);
	}
	if (close(parameters->fifo_input) == -1)
	{
		perror("Closing fifo /tmp/rejeitados");
		exit(1);
	}
	if (unlink("/tmp/rejeitados"))
	{
		perror("Closing fifo fifo /tmp/rejeitados");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Incorrect number of arguments\n");
		exit(1);
	}

	ParametersStruct parameters;
	parameters.init_time = GetTimeSinceProgramStartup(0);
	MAX_LUGARES = atoi(argv[1]);
	LUGARES_LIVRES = MAX_LUGARES;

	GENDER = 'N';//N represents no gender defined (i.e nobody in the sauna)

	MakeAndOpenFifos(&parameters);
	parameters.log_filedes = InitLogFile();

	Statistics stats;
	InitializeStats(&stats);

	parameters.stats = &stats;
	parameters.tids = new_vector_tid();
	Request request;
	pid_t pid = getpid();
	pthread_t tid = pthread_self();

	parameters.request = &request;
	while (read(parameters.fifo_input, &request, sizeof(Request)) != 0)
	{
		double inst = GetTimeSinceProgramStartup(parameters.init_time);
		WriteToLogFile(&request, inst, parameters.log_filedes, pid, tid, "RECEBIDO");
		IncrementReceivedRequests(&stats, parameters.request->gender);
		HandleRequest(&parameters, &stats);
	}

	for (int i = 0; i < parameters.tids->size; i++)
	{
		if (pthread_join(get_tid_at(parameters.tids, i), NULL) != 0)
		{
			printf("Error Joining thread\n");
			exit(1);
		}
	}

	CloseAndDeleteFifos(&parameters);

	PrintStatistics(&stats);

	return 0;
}