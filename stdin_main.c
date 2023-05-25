#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // for sysconf - # of processors

#include <pthread.h>

#define MAX_ENCRYPTED_SIZE 1024
#define SIZE_OF_TASK_QUEUE 16

void encrypt(char *s,int key) {
	int i;
	for (i = 0; i < strlen(s); i++)
	{
		s[i] = s[i] + key;
	}
	// sleep(1);
}

void decrypt(char *s,int key) {
	int i;
	for (i = 0; i < strlen(s); i++)
	{
		s[i] = s[i] - key;
	}
	// sleep(1);
}

char* decrypt_copy(char *s,int key) {
	char *copy = malloc(sizeof(char) * strlen(s));
	strcpy(copy, s);
	decrypt(copy, key);
	return copy;
}

typedef struct task {
	char *data;
	int index;
	int isDone;
	int isStarted;
} task;



int outputDone;
int key;
int running;
int active_threads;
long number_of_processors;
int amount_of_tasks = 0;
int encryptFlag = 1;
int queue_size;
task **task_queue;
pthread_mutex_t outputLock;
pthread_mutex_t activeThreadsMutex;

void modify_active_threads(int change) {
	pthread_mutex_lock(&activeThreadsMutex);
	active_threads += change;
	pthread_mutex_unlock(&activeThreadsMutex);
}


void enlarge_task_queue() {
	pthread_mutex_lock(&outputLock);
	task_queue = realloc(task_queue, sizeof(task) * queue_size * 2);
	pthread_mutex_unlock(&outputLock);
	if (task_queue == NULL)
	{
		printf("Error: failed to enlarge task_queue\n");
		exit(0);
	}
	queue_size *= 2;
}

void* encrypt_task(void *arg) {
	task *currTask = (task*)arg;
	int indexBefore = currTask->index;

	if (encryptFlag)
		encrypt(currTask->data, key);
	else
		decrypt(currTask->data, key);

	if (indexBefore != currTask->index)
	{
		printf("index changed during encryption! from %d to %d\nEncrypting function goes out of its bounds!\n!CHECK YOUR ENCRYPTION FUNCTION!\n", indexBefore, currTask->index);
		exit(1);
	}
	currTask->isDone = 1;
	// active_threads--;
	modify_active_threads(-1);

	return 0;
}

void create_new_thread(task *currTask) {
	currTask->isStarted = 1;
	pthread_t encrypt_thread;
	pthread_create(&encrypt_thread, NULL, encrypt_task, (void*)currTask);
	// active_threads++;
	modify_active_threads(1);
}

void* output_and_threads_manager() {
	int highest_index_printed = -1;
	int allPrinted = 1;
	int RUN = 1;
	while (RUN) {
		// running || allPrinted
		// check if there is a new task and a free thread to deal with it

		allPrinted = 1;
		// iterate over all task_queue elements and check if the next is ready to print
		for (int i = 0; i < amount_of_tasks; i++)
		{
			// pthread_mutex_lock(&outputLock);
			// first check if task_queue[i] is not null
			if (i >= queue_size || task_queue[i] == NULL)
			{
				continue;
			}
			if (task_queue[i]->isStarted == 0 && active_threads < number_of_processors)
			{
				create_new_thread(task_queue[i]);
			}
			else if (task_queue[i]->isDone && task_queue[i]->index == highest_index_printed + 1)
			{
				// print the data
				// decrypt(task_queue[i]->data, key);
				printf("%s",task_queue[i]->data);
				// printf("\n%d\n", task_queue[i]->index);
				// fflush(stdout);
				// printf("\n%d\n", task_queue[i]->index);
				// increment highest_index_printed
				highest_index_printed++;
				// free the task_queue[i]
				free(task_queue[i]->data);
				free(task_queue[i]);
				// set task_queue[i] to null
				task_queue[i] = NULL;
				allPrinted = 0;
			}
			// pthread_mutex_unlock(&outputLock);
		}
		if (!running && !allPrinted && highest_index_printed == amount_of_tasks - 1) {
			RUN = 0;
		}
	}
	// printf("output_thread terminated with: highest_index_printed: %d,  \n", highest_index_printed);
	// active_threads--;
	modify_active_threads(-1);
	outputDone = 1;
	return 0;
}



task* create_new_task(char *currData, int counter) {
	task *currTask = malloc(sizeof(task));
	currTask->data = malloc(sizeof(char) * (counter+1));
	strncpy(currTask->data, currData, counter);
	currTask->index = amount_of_tasks;
	currTask->isDone = 0;
	currTask->isStarted = 0;
	if (amount_of_tasks == queue_size)
	{
		enlarge_task_queue();
	}
	task_queue[amount_of_tasks] = currTask;
	amount_of_tasks++;
	return currTask;
}

void input_manager() {
	char c;
	int counter = 0;
	char currData[MAX_ENCRYPTED_SIZE]; 
	while ((c = getchar()) != EOF)
	{
	  currData[counter] = c;
	  counter++;
	  if (counter == MAX_ENCRYPTED_SIZE){
		// This is where we check how many threads are active, 
		// if there are less than the number of processors, then we create a new thread with the task of encrypting the data
		// otherwise, we store the data in a queue and wait for a thread to finish
		currData[counter] = '\0';
		task *currTask = create_new_task(currData, counter);
		//if (active_threads < number_of_processors) 
		//	create_new_thread(currTask);
		// encrypt(currData,key);
		// printf("encripted currData: %s\n",currData);
		counter = 0;
	  }
	}
	if (counter > 0)
	{
		currData[counter] = '\0';
		task *currTask = create_new_task(currData, counter);
		//currTask->data[counter] = '\0';
		//create_new_thread(currTask);
	}
}



int main(int argc, char *argv[])
{
	if (argc < 2)
	{
	    printf("usage: ./tester key < file \n");
	    return 0;
	}
	key = atoi(argv[1]);
	if (argc == 3) {
		if (strcmp(argv[2], "-d") == 0) {// decrypt 
			encryptFlag = 0;
		} else { // -e or anything else lol
			encryptFlag = 1; // encrypt
		}
	}
	number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
	if (number_of_processors <= 2)
	{
		printf("Error: number of processors is less than or equal to 2\n");
		return 0;
	}
	queue_size = SIZE_OF_TASK_QUEUE;
	task_queue = malloc(sizeof(task) * queue_size);
	active_threads = 1;
	outputDone = 0;
	amount_of_tasks = 0;
	outputLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	// initialize and start a thread on output_manager function:
	pthread_t output_and_threads_manager_thread;
	running = 1;
	pthread_create(&output_and_threads_manager_thread, NULL, output_and_threads_manager, NULL);
	// active_threads++;
	modify_active_threads(1);
	input_manager();
	running = 0;
	// active_threads--;
	modify_active_threads(-1);

	// wait for all threads to finish
	while (!outputDone)
	{
		// printf("finishing..: %d\n", active_threads);
	}
	// printf("finished with %d tasks\n", amount_of_tasks);
	free(task_queue);
	
	return 0;
}
