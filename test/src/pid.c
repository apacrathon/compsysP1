/*
 * system.c
 *
 *  Created on: Feb 21, 2017
 *      Author: raj
 */
#include <stdlib.h>

int print_pid();
int sys();

int main(void)
{
	sys();
}

int print_pid()
{
	printf("The process ID is %d\n", (int) getpid());
	printf("The parent process ID %d\n", (int) getppid());
	return 0;
}

int sys()
{
	int return_value = system("ls -l /");
	return return_value;
}


