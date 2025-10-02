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


int main(int argc, char *argv[])
{
    struct stat buf;
    unsigned short cksum1, cksum2;
    uint8_t  *shm_ptr;
    int      shm_fd;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <shm_name> \n", argv[0]);
        return -1;
    }

    char *shm_name = argv[1];

    /* open the shared memory segment */
    shm_fd = shm_open(shm_name, O_RDONLY, 0644);
    if (shm_fd == -1) {
        fprintf(stderr, "Error unable to open shared memory segment, '%s, errno = %d (%s)\n", shm_name,
                errno, strerror(errno));
        return -1;
    }

    /* get configuration of shared memory segment */
    if (fstat(shm_fd, &buf) == -1) {
        fprintf(stderr, "Error unable to status shared memory segment fd = %d, errno = %d (%s)\n", shm_fd,
                errno, strerror(errno));
        return -1;
    }

    /* now map the shared memory segment in the address space of the process */
    shm_ptr = mmap(0, buf.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        fprintf(stderr, "Error: unable to map shared memory segment, errno = %d (%s) \n",
                errno, strerror(errno));
        return -1;
    }

    /* check checksum data from shared memory */
    memcpy(&cksum1, &shm_ptr[buf.st_size-2], 2);

    /* calculate checksum */
    cksum2 = checksum(&shm_ptr[0], buf.st_size-2);

    /* Compare checksums */
    if (cksum1 != cksum2) {
       printf("Checksum mismatch: expected %02x, received %02x \n", cksum2, cksum1);
       return -1;
    }

    printf("Checksums match !!! \n");

    return 0;

} 
