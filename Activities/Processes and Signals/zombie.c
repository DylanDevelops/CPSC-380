/**
 * zombie.c
 *
 * This small program generates a zombie process.
 *
 * A zombie process is created when a process terminates,
 * but the parent does not invoke wait(), but the parent
 * continues to run. By not invoking wait(), the child
 * process maintains an entry in the process entry table
 * as well as maintaining its pid.
 *
 * Pretty simple to do:
 * 
 * (1) Create a child process
 * (2) Have the parent run for a specified amount of time.
 *
 * If this program is run in the background, the state of the child
 * process illustrates it is a zombie process.
 * (Z underneath the S field using ps) 
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(void)
{
	pid_t pid;

	pid = fork();

	if (pid < 0) {
		fprintf(stderr,"Unable to create child process\n");

		return -1;
	}
	else if (pid == 0) {
		/**
		 * Just have the child exit
		 * and turn into a zombie.
		 */

		return 0;
	}
	else {
		/**
		 * Have the parent sleep for 10 seconds
		 */

		sleep(10);

		printf("Parent exiting\n");

		return 0;
	}
}

