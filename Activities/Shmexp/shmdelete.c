#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>


int main(int argc, char **argv)
{

    char *shm_name;
    

    if (argc != 2) {
        printf("Usage: %s <shm-name> \n", argv[0]);
        return -1;
    }

    shm_name = argv[1];

    /* remove the shared memory segment */
    if (shm_unlink(shm_name) == -1) {
        fprintf(stderr, "Error unable to remove shared memory segment '%s', errno = %d (%s) \n", shm_name,
                errno, strerror(errno));
        return -1;
    }


    return 0;
}

