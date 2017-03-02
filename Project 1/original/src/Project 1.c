/*
 ============================================================================
 Name        : Project.c
 Author      : Raj Patel
 Version     :
 Copyright   : © 2017 Apacrathon, All Rights Reserved.
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#define SHM_SIZE (4 * sizeof(int))  /* 4 integer memory segment size*/

static int32_t NUM_CHILDREN = 10;
static FILE * output;
/*
 * Functions to read and write to a file.
 */
int32_t write_random_nums(int32_t n, FILE * f);
int32_t read_file(int32_t n, int32_t data[], FILE * f);

/*
 * Arithmetic functions for an array
 */

int32_t sum_of_array(int32_t n, int32_t data[]);
int32_t min_of_array(int32_t n, int32_t data[]);
int32_t max_of_array(int32_t n, int32_t data[]);

/*
 * Functions for each part
 */

int32_t PART_A(int32_t data_size, int32_t data[]);
int32_t PART_B(int32_t data_size, int32_t data[]);
int32_t PART_C(int32_t data_size, int32_t data[]);
int32_t PART_D(int32_t data_size, int32_t data[]);
int32_t PART_EC(int32_t data_size, int32_t data[]);

/*
 * Misc. functions.
 */
void print_array(int32_t data_size, int32_t data[]);

int main(void)
{
	srand(time(NULL));

	FILE * f_write = fopen("random.txt", "w");
	FILE * f_read = fopen("random.txt", "r");
	int32_t data_size = 10000, data[data_size];

	//printf("\nWriting to file...\n");
	write_random_nums(data_size, f_write);
	//printf("Successfully wrote to file...\n");

	//printf("Reading from file...\n");
	read_file(data_size, data, f_read);
	//printf("Successfully read from file...\n");

	PART_A(data_size, data);
	PART_B(data_size, data);
	PART_C(data_size, data);
	PART_D(data_size, data);

	return EXIT_SUCCESS;
}

int32_t PART_A(int32_t data_size, int32_t data[])
{
	output = fopen("output_part_a.txt", "w");
	clock_t begin = clock();
	/* here, do your time-consuming job */

	pid_t pid = getpid();
	pid_t ppid = getppid();

	printf("\n-------------------Running Part A-------------------\n");
	printf("Hi, I'm process %d, and my parent is %d.\n", pid, ppid);
	fprintf(output, "Hi, I'm process %d, and my parent is %d.\n", pid, ppid);
	printf("Max = %d\n", max_of_array(data_size, data));
	fprintf(output, "Max = %d\n", max_of_array(data_size, data));
	printf("Min = %d\n", min_of_array(data_size, data));
	fprintf(output, "Min = %d\n", min_of_array(data_size, data));
	printf("Sum = %d\n", sum_of_array(data_size, data));
	fprintf(output, "Sum = %d\n", sum_of_array(data_size, data));
	fclose(output);

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Time: %lf", time_spent);
	printf("\n-------------------Ending Part A-------------------\n");
	return 0;
}

int32_t PART_B(int32_t data_size, int32_t data[])
{
	printf("\n-------------------Running Part B-------------------\n");
	output = fopen("output_part_b.txt", "w");

	clock_t begin = clock();
	int32_t min = data[0], max = data[0], sum = 0, count = 0,
			num_processes, data_per_process,
			shmid_write, shmid_read;
	int32_t pipefd[2], data_buffer[data_size], * stats,
			NUM_CHILD = NUM_CHILDREN, total_processes = NUM_CHILD;
	key_t key = ftok("random.txt", 'aa');

	/*
	 * Get shmid (writing) and attach system to shared segment.
	 * Write inital stats to shared memory.
	 */
	//wait(NULL);
	if ((shmid_write = shmget (key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { perror("shmget :failed"); exit(1); }
	else { /*(void) fprintf(stderr, "shmget: returned %d\n", shmid_write);*/ }
	stats = (int *) shmat(shmid_write, NULL, 0);
	stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

	// Used to determine process/work division
	if (total_processes > data_size)
	{
		total_processes = data_size;
		NUM_CHILD = total_processes;
	}

	data_per_process = ceil((double)data_size / total_processes);

	// Create pipe, send data array from parent end, and close the parent write end.
	if (pipe(pipefd) == -1) { printf("Error creating pipe."); exit(1); }
	write(pipefd[1], data, data_size*sizeof(int));
	close(pipefd[1]);

	pid_t pid;
	pid = fork ();
	if (pid > 0)
	{
		int dummy;
		wait(&dummy);
	}
	else if (pid == 0)
	{
		printf ("Hi I'm process %d and my parent is %d.\n",getpid (), getppid ());
		fprintf (output, "Hi I'm process %d and my parent is %d.\n",getpid (), getppid ());
		fclose(output);
		int32_t newData;
		if (count >= data_size)
		{
			newData = data_per_process - ((data_per_process * total_processes) - data_size);
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}
		else
		{
			newData = data_per_process;
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}

		/*
		 * Get shmid (reading) and attach system to shared segment.
		 * Read current stats from shared memory.
		 */
		if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
		stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
		min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

		// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
		for (int j = 0; j < data_per_process; j++)
		{
			count++;
			if (min > data_buffer[j]) { min = data_buffer[j]; }
			if (max < data_buffer[j]) { max = data_buffer[j]; }
			sum += data_buffer[j];
			if (count >= data_size) { break; }
		}

		/*
		 * Get shmid (writing) and attach system to shared segment.
		 * Write updated stats to shared memory.
		 */
		//wait(NULL);
		if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
		stats = (int *) shmat(shmid_write, NULL, 0);
		stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;
		newFork(0, NUM_CHILD - 1, &pipefd, data, data_size, key, &stats);

		exit(0);
	}
	else { printf("fork error\n"); return -1; }

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);
	if (pid > 0)
	{
		output = fopen("output_part_b.txt", "a");
		fprintf(output, "Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);
		fclose(output);
	}
	shmdt((void *) stats);
	printf("-------------------Ending Part B-------------------\n");

	return 0;
}

int32_t PART_C(int32_t data_size, int32_t data[])
{
	printf("\n-------------------Running Part C-------------------\n");
	output = fopen("output_part_c.txt", "w");
	/*
	 * min, max, data 	 				used for statistics
	 * num_process, data_per_process  	used for determining number of processes
	 * shmid_write, shmid_read			stores shmids for reading and writing
	 * pipefd 							used to pass data array to children
	 * data_buffer  					used as buffer for pipefd
	 * stats  							shared memory segment for min, max, and sum.
	 */
	clock_t begin = clock();
	int32_t min = data[0], max = data[0], sum = 0, count = 0,
			num_processes, data_per_process,
			shmid_write, shmid_read,
			NUM_CHILD = NUM_CHILDREN, total_processes = NUM_CHILD;
	int32_t pipefd[2], data_buffer[data_size], * stats;
	key_t key = ftok("random.txt", 'rr');

	/*
	 * Get shmid (writing) and attach system to shared segment.
	 * Write inital stats to shared memory.
	 */
	if ((shmid_write = shmget (key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { perror("shmget :failed"); exit(1); }
	else { /*(void) fprintf(stderr, "shmget: returned %d\n", shmid_write);*/ }
	stats = (int *) shmat(shmid_write, NULL, 0);
	stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

	// Used to determine process/work division


	//	if (ceil((double) data_size / total_processes) > (data_size / total_processes)) { NUM_CHILD++, total_processes = NUM_CHILD * NUM_GRANDCHILDREN; }
	data_per_process = ceil((double)data_size / total_processes);

	// Create pipe, send data array from parent end, and close the parent write end.
	if (pipe(pipefd) == -1) { printf("Error creating pipe."); exit(1); }
	write(pipefd[1], data, data_size*sizeof(int));
	close(pipefd[1]);

	// Spawn num_processes processes for the parent process.
	for (int i = 0; i < NUM_CHILD; i++)
	{
		pid_t pid = fork();
		if (pid > 0)	// in parent
		{
			int dummy;
			wait(&dummy);		// wait for the child to finish
		}
		else if (pid == 0)		// in child
		{

			printf("Hi, I'm process %d, and my parent is %d\n", getpid(), getppid());
			fprintf (output, "Hi I'm process %d and my parent is %d.\n",getpid (), getppid ());
			fclose(output);

			int32_t newData;
			if (count >= data_size)
			{
				newData = data_per_process - ((data_per_process * total_processes) - data_size);
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}
			else
			{
				newData = data_per_process;
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}


			/*
			 * Get shmid (reading) and attach system to shared segment.
			 * Read current stats from shared memory.
			 */
			if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
			stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
			min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

			// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
			for (int j = 0; j < newData; j++)
			{
				count++;
				if (min > data_buffer[j]) { min = data_buffer[j]; }
				if (max < data_buffer[j]) { max = data_buffer[j]; }
				sum += data_buffer[j];
				if (count >= data_size) { break; }
				//else { count++; }
			}

			/*
			 * Get shmid (writing) and attach system to shared segment.
			 * Write updated stats to shared memory.
			 */
			if ((shmid_write = shmget (key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
			wait(NULL);
			stats = (int *) shmat(shmid_write, NULL, 0);
			stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

			exit(0);
		}
		else { printf("fork error\n"); return -1; }
	}

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);
	output = fopen("output_part_c.txt", "a");
	fprintf(output, "Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);
	fclose(output);

	shmdt(stats);

	printf("-------------------Ending Part C-------------------\n");
	return 0;
}

int32_t PART_D(int32_t data_size, int32_t data[])
{
	printf("\n-------------------Running Part D-------------------\n");
	output = fopen("output_part_d.txt", "w");
	clock_t begin = clock();

	int32_t min = data[0], max = data[0], sum = 0, count = 0,
			num_processes, data_per_process,
			shmid_write, shmid_read;
	int32_t pipefd[2], data_buffer[data_size], * stats,
			NUM_GRANDCHILDREN = 4, NUM_CHILD = 2, total_processes = 6;

	key_t key = ftok("random.txt", 'ww');

	/*
	 * Get shmid (writing) and attach system to shared segment.
	 * Write inital stats to shared memory.
	 */
	if ((shmid_write = shmget (key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { perror("shmget :failed"); exit(1); }
	else { /*(void) fprintf(stderr, "shmget: returned %d\n", shmid_write);*/ }
	stats = (int *) shmat(shmid_write, NULL, 0);
	stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

	if (data_size <= 10)
	{
		data_per_process = 1;
	}
	else
	{
		data_per_process = ceil((double)data_size / total_processes);
	}


	//printf("data per processes: %d, child: %d, gchild: %d, total_p: %d\n", data_per_process, NUM_CHILD, NUM_GRANDCHILDREN, total_processes);

	// Create pipe, send data array from parent end, and close the parent write end.
	if (pipe(pipefd) == -1) { printf("Error creating pipe."); exit(1); }
	write(pipefd[1], data, data_size*sizeof(int));
	close(pipefd[1]);


	for (int i = 0; i < NUM_CHILD; i++)
	{
		pid_t pid = fork();

		if (pid > 0)
		{
			int dummy;
			wait(&dummy);
		}
		else if (pid == 0)
		{
			//printf("Hi I'm process %d, and my parent is %d.\n", getpid(),getppid());
			int32_t newData;
			if (count >= data_size)
			{
				newData = data_per_process - ((data_per_process * total_processes) - data_size);
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}
			else
			{
				newData = data_per_process;
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}

			/*
			 * Get shmid (reading) and attach system to shared segment.
			 * Read current stats from shared memory.
			 */
			if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
			stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
			min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

			// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
			for (int j = 0; j < newData; j++)
			{
				count++;
				if (min > data_buffer[j]) { min = data_buffer[j]; }
				if (max < data_buffer[j]) { max = data_buffer[j]; }
				sum += data_buffer[j];
				if (count >= data_size) { break; }
				///else { count++; }
			}

			/*
			 * Get shmid (writing) and attach system to shared segment.
			 * Write updated stats to shared memory.
			 */
			if ((shmid_write = shmget (key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
			stats = (int *) shmat(shmid_write, NULL, 0);
			stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

			newFork2(0, 1, &pipefd, data, data_size, key, &stats);
			wait(NULL);
			exit(0);
		}
		else { printf("fork error\n"); return -1; }
	}
	//sleep(2);
	wait(NULL);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);

	output = fopen("output_part_d.txt", "a");
	fprintf(output, "Max = %d\nMin = %d\nSum = %d\nTime: %lf\n", stats[1], stats[0], stats[2], time_spent);
	fclose(output);

	shmdt((void *) stats);
	printf("-------------------Ending Part D-------------------\n");
	return 1;
}

void newFork(int32_t i, int32_t n, int32_t * pipefd, int32_t data[], int32_t data_size, key_t key, int32_t * stats)
{
	int32_t min = data[0], max = data[0], sum = 0, count = 0,
				num_processes, data_per_process,
				shmid_write, shmid_read,
				NUM_CHILD = NUM_CHILDREN, total_processes = NUM_CHILD;
	int32_t data_buffer[data_size];

	/*
	 * Get shmid (writing) and attach system to shared segment.
	 * Write inital stats to shared memory.
	 */
	if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget :failed"); exit(1); }
	else { /*(void) fprintf(stderr, "shmget: returned %d\n", shmid_write);*/ }
	stats = (int *) shmat(shmid_write, NULL, 0);

	// Used to determine process/work division

	data_per_process = ceil((double)data_size / total_processes);

	if(i >= n) { return; }
	pid_t pid = fork();
	if(pid > 0)
	{
		wait(NULL);
	}
	else if (pid == 0)
	{
		printf ("Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		output = fopen("output_part_b.txt", "a");
		fprintf (output, "Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		fclose(output);

		int32_t newData;
		if (count >= data_size)
		{
			newData = data_per_process - ((data_per_process * total_processes) - data_size);
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}
		else
		{
			newData = data_per_process;
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}

		/*
		 * Get shmid (reading) and attach system to shared segment.
		 * Read current stats from shared memory.
		 */
		if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
		stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
		min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

		// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
		for (int j = 0; j < data_per_process; j++)
		{
			count++;
			if (min > data_buffer[j]) { min = data_buffer[j]; }
			if (max < data_buffer[j]) { max = data_buffer[j]; }
			sum += data_buffer[j];
			if (count >= data_size) { break; }
			//else { count++; }
		}

		/*
		 * Get shmid (writing) and attach system to shared segment.
		 * Write updated stats to shared memory.
		 */
		wait(NULL);
		if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
		stats = (int *) shmat(shmid_write, NULL, 0);
		stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;
		i++;
		newFork(i, NUM_CHILD - 1, pipefd, data, data_size, key, stats);
	}
	else { printf("fork error\n"); return; }
}

void newFork2(int32_t i, int32_t n, int32_t * pipefd, int32_t data[], int32_t data_size, key_t key, int32_t * stats)
{
	int32_t min = data[0], max = data[0], sum = 0, count = 0,
				num_processes, data_per_process, NUM_GRANDCHILDREN = 4, NUM_CHILD = 2, total_processes = 6,
				shmid_write, shmid_read;
	int32_t data_buffer[data_size];

	/*
	 * Get shmid (writing) and attach system to shared segment.
	 * Write inital stats to shared memory.
	 */
	if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget :failed"); exit(1); }
	else { /*(void) fprintf(stderr, "shmget: returned %d\n", shmid_write);*/ }
	stats = (int *) shmat(shmid_write, NULL, 0);

	data_per_process = ceil((double)data_size / total_processes);
	//printf("data per processes: %d\n", data_per_process);


	if(i >= n) { return; }

	pid_t pid = fork();
	if(pid < 0) {}
	else if (pid == 0)
	{
		printf ("Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		output = fopen("output_part_d.txt", "a");
		fprintf (output, "Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		fclose(output);
		int32_t newData;
		if (count >= data_size)
		{
			newData = data_per_process - ((data_per_process * total_processes) - data_size);
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}
		else
		{
			newData = data_per_process;
			read(pipefd[0], data_buffer, newData*sizeof(int));
		}

		/*
		 * Get shmid (reading) and attach system to shared segment.
		 * Read current stats from shared memory.
		 */
		if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
		stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
		min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

		// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
		for (int j = 0; j < newData; j++)
		{
			count++;
			if (min > data_buffer[j]) { min = data_buffer[j]; }
			if (max < data_buffer[j]) { max = data_buffer[j]; }
			sum += data_buffer[j];
			if (count >= data_size) { break; }
		}


		/*
		 * Get shmid (writing) and attach system to shared segment.
		 * Write updated stats to shared memory.
		 */
		if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
		else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
		stats = (int *) shmat(shmid_write, NULL, 0);
		stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;

		//sleep(2);

		i++;
		//newFork2(i, NUM_GRANDCHILDREN, pipefd, data, data_size, key, stats);
	}
	else if (pid > 0)
	{
		printf ("Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		output = fopen("output_part_d.txt", "a");
		fprintf (output, "Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
		fclose(output);

		pid_t pid2 = fork();
		if (pid2 == 0)
		{
			printf ("Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
			output = fopen("output_part_d.txt", "a");
			fprintf (output, "Hi I'm process %d and my parent is %d.\n", getpid (), getppid ());
			fclose(output);

			int32_t newData;
			if (count >= data_size)
			{
				newData = data_per_process - ((data_per_process * total_processes) - data_size);
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}
			else
			{
				newData = data_per_process;
				read(pipefd[0], data_buffer, newData*sizeof(int));
			}

			/*
			 * Get shmid (reading) and attach system to shared segment.
			 * Read current stats from shared memory.
			 */
			if ((shmid_read = shmget (key, SHM_SIZE, IPC_EXCL)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_read);*/ }
			stats = (int *) shmat(shmid_read, NULL, SHM_RDONLY);
			min = stats[0], max = stats[1], sum = stats[2], count = stats[3];

			// Have the child check its assigned chunk of data to see if there is a new min/max, and update the sum
			for (int j = 0; j < newData; j++)
			{
				count++;
				if (min > data_buffer[j]) { min = data_buffer[j]; }
				if (max < data_buffer[j]) { max = data_buffer[j]; }
				sum += data_buffer[j];
				if (count >= data_size) { break; }
			}

			/*
			 * Get shmid (writing) and attach system to shared segment.
			 * Write updated stats to shared memory.
			 */
			if ((shmid_write = shmget (key, SHM_SIZE, IPC_CREAT)) == -1) { perror("shmget: shmget failed"); exit(1); }
			else { /*(void) fprintf(stderr, "shmget: shmget returned %d\n", shmid_write);*/ }
			stats = (int *) shmat(shmid_write, NULL, 0);
			stats[0] = min, stats[1] = max, stats[2] = sum, stats[3] = count;
		}
	}
	else { printf("fork error\n"); return; }
}

int32_t write_random_nums(int32_t n, FILE * f)
{

	int32_t result = 0;

	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

	for (int count = 0; count < n; count++)
	{
		fprintf(f, "%d\n", (rand() % 1000));
	}

	result = fclose(f);

	return result;
}

int32_t read_file(int32_t n, int32_t data[], FILE * f)
{
	int32_t result = 0;

	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    return 1;
	}

	for (int i = 0; i < n; i++)
	{
		fscanf(f, "%d", &data[i]);
	}

	result = fclose(f);

	return result;
}

int32_t sum_of_array(int32_t n, int32_t data[])
{
	int sum = 0;

	for (int i = 0; i < n; i++)
	{
		sum += data[i];
	}

	return sum;
}

int32_t min_of_array(int32_t n, int32_t data[])
{
	int min = data[0];

	for (int i = 1; i < n; i++)
	{
		if (min > data[i]) { min = data[i]; }
	}

	return min;
}

int32_t max_of_array(int32_t n, int32_t data[])
{
	int min = data[0];

	for (int i = 1; i < n; i++)
	{
		if (min < data[i]) { min = data[i]; }
	}

	return min;
}

void print_array(int32_t n, int32_t data[])
{
	for (int32_t i = 0; i < n; i++)
	{
		printf("%d\n", data[i]);
	}
}