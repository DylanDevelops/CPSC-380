/** 
 * Full Name: Dylan Ravel and Daniel Tsivkovski
 * Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 2 - Thread Synchronization
*/

#include "buffer.h"
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

/* the buffer */ 
BUFFER_ITEM buffer[BUFFER_SIZE];

int in = 0; // indices to put in
int out = 0; // indices for taking out

pthread_mutex_t mutex; 
sem_t empty; // num of empty spots
sem_t full; // num of full spots

// producer thread function
void *producer(void *param) {
    BUFFER_ITEM item;
    while (1) {
        /* release the CPU randomly to simulate preemption */
        if ((rand() % 100) < 40) { // ~40% chance
            sched_yield(); // voluntarily yield CPU
        }
        /* generate the item */
        if (insert_item(item))
        fprintf("report error condition");
    }
}

// consumer thread function
void *consumer(void *param) {
    BUFFER_ITEM item;
    while (true) {
        /* release the CPU randomly to simulate preemption */
        if ((rand() % 100) < 40) { // ~40% chance
            sched_yield(); // voluntarily yield CPU
        }

        if (remove_item(&item))
            fprintf("report error condition");

        /* verify the item removed matches the item inserted */
    }
}

int insert_item(BUFFER_ITEM item) {
    /* insert item into buffer return O if successful, otherwise return -1 indicating an error condition */
    
}

int remove_item(BUFFER_ITEM *item) {
    /* remove an object from buffer placing it in item return O if successful, otherwise return -1 indicating an error condition */
}

int main (int argc, char *argv[]) {
    /* 1. Get command line arguments argv[1], argv[2], argv [3] */
    /* 2. Initialize buffer */
    /* 3. Create producer thread(s) */ 
    /* 4. Create consumer thread(s) */
    /* 5. Sleep */ 
    /* 6. Exit */
}
