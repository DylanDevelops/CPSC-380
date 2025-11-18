/** 
 * Full Name: Dylan Ravel
 * Chapman Email: ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: InClass - Disk Scheduler
*/

#include <stdio.h>
#include <stdlib.h>

int requests[] = {2069, 1212, 2296, 2800, 544, 1618, 356, 1523, 4965, 3681};

int main(int argc, char* argv[])
{
    int totalMovement = 0;

    if(argc != 2) {
        printf("Usage: %s <starting_position>\n", argv[0]);
        return -1;
    }

    int initialPosition = atoi(argv[1]);

    if(initialPosition < 0 || initialPosition > 4999) {
        printf("Invalid Position: %d\n", initialPosition);
        return -1;
    }

    for(int i = 0; i < 10; ++i) {
        totalMovement += abs(requests[i] - initialPosition);
        initialPosition = requests[i];
    }

    printf("Total Movement: %d\n", totalMovement);

    return 0;
}