#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

extern uint16_t checksum(char* addr, uint32_t count);

int main(int argc, char *argv[])
{
    uint8_t  *shm_ptr;
    uint16_t cksum;
    struct stat buf;

    int i;
    int shm_fd;
    char *shm_name;

    if (argc != 2) {
        fprintf(stderr, "%s:: Usage: <shm_name> \n", argv[0]);
        return -1;
    }

    shm_name = argv[1];

    /* open the shared memory memory segment */
    shm_fd = shm_open(shm_name, O_RDWR, 0644);
    if (shm_fd == -1) {
        fprintf(stderr, "Error unable to create shared memory, '%s, errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
        return -1;
    }

    /* get configuration of shared memory segment */
    if (fstat(shm_fd, &buf) == -1) {
        fprintf(stderr, "Error unable to status shared memory segment fd = %d, errno = %d (%s)\n", shm_fd,
                errno, strerror(errno));
        return -1;
    }

    /* attach to shared memory region */
    shm_ptr = (uint8_t *)mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        fprintf(stderr, "Error: unable to map shared memory segment, errno = %d (%s) \n", 
                errno, strerror(errno));
        return -1;
    }

    /* write random data to shared memory */
    for (i = 0; i < buf.st_size-2; i++) {
       shm_ptr[i] = (unsigned char) (rand() % 256);
    }

    /* calculate checksum */
    cksum = checksum(&shm_ptr[0], buf.st_size-2);

    /* copy checksum to shared memory */
    memcpy(&shm_ptr[buf.st_size-2], &cksum, 2);

    return 0;
} 
