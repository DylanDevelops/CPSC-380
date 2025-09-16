#include <sys/types.h>
#include <wait.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main()
{

    pid_t  pid;

    /* fork the child process */
    pid = fork();

    if (pid < 0) { /* error occured */
        printf("fork() failed: %d (%s) \n", errno, strerror(errno));
        return -1;
    }
    else if (pid == 0) { /* child process */
        execlp("/bin/ls","ls",NULL);
    }
    else { /* parent process */
        /* parent will wait for the child to complete */
        wait(NULL);
        printf("Child Complete \n");
    }

    return 0;

}
    
