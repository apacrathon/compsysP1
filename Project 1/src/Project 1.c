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

int32_t write_random_nums(int32_t n, char file_name);

int main(void) {
	srand(time(NULL));

	write_random_nums(10, "file.txt");

	return EXIT_SUCCESS;
}

int32_t write_random_nums(int32_t n, char file_name)
{

	int32_t result = 0;
	//if (strcmp(file_name, "") == 0) { file_name = "random.list"; }

	FILE * rand_num_file = fopen("random.list", "w");
	if (rand_num_file == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

	for (int count = 0; count < n; count++)
	{
		fprintf(rand_num_file, "%d\n", (rand() % 100));
	}

	result = fclose(rand_num_file);

	return result;
}

void