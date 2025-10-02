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
    void *shm_ptr;

    int size;
    int shm_fd;
    char *shm_name;
    

    if (argc != 3) {
        printf("Usage: %s <shm-name> <shm-size>\n", argv[0]);
        return -1;
    }

    shm_name = argv[1];
    size = atoi(argv[2]);

    /* create the shared memory segment */
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        fprintf(stderr, "Error unable to create shared memory, '%s, errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
        return -1;
    }

    /* configure the size of the shared memory segment */
    if (ftruncate(shm_fd, size) == -1) {
         fprintf(stderr, "Error configure create shared memory, '%s, errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
         shm_unlink(shm_name);
         return -1;
    }

    printf("shared memory create success, shm_fd = %d\n", shm_fd);

    return 0;
}

