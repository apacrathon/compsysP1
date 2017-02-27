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

/*
 * Functions to read and write to a file.
 */
int32_t write_random_nums(int32_t n, FILE * f);
int32_t read_file(int32_t n, int32_t data[], FILE * f);


int32_t sum_of_array(int32_t n, int32_t data[]);
int32_t min_of_array(int32_t n, int32_t data[]);

int main(void)
{
	srand(time(NULL));

	FILE * f_write = fopen("random.list", "w");
	FILE * f_read = fopen("random.list", "r");
	int32_t data_size = 10, data[data_size];

	write_random_nums(data_size, f_write);
	printf("Successfully wrote to file...\n");
	printf("Reading from file...\n");
	read_file(data_size, data, f_read);
	printf("Successfully read from file...\n");

	for (int32_t i = 0; i < data_size; i++)
	{
		printf("%d\n", data[i]);
	}

	return EXIT_SUCCESS;
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
		fprintf(f, "%d\n", (rand() % 100));
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
		if (min < data[i]) { min = data[i]; }
	}

	return min;
}
