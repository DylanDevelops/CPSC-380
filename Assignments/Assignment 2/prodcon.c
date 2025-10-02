/** 
 * Full Name: Dylan Ravel and Daniel Tsivkovski
 * Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 2 - Thread Synchronization
*/

#include "buffer.h"
#include <stdio.h>
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

// calculates the checksum using arbitrary values.
uint16_t calculateChecksum(uint8_t data[30]) {
    uint16_t cksum = 0;
    for (int i = 0; i < 30; ++i) {
        cksum += data[i] * (i % 10);
    }
    return cksum;
}

// producer thread function
void *producer(void *param) {
    BUFFER_ITEM item;
    while (1) {
        /* release the CPU randomly to simulate preemption */
        if ((rand() % 100) < 40) { // ~40% chance
            sched_yield(); // voluntarily yield CPU
        }

        // generate random data
        for (int i = 0; i < 30; ++i) {
            item.data[i] = rand() % 256;
        }

        // calculate checksum
        item.cksum = calculateChecksum(item.data);

        // check if item was not inserted correctly
        if (insert_item(item))
            fprintf(stderr, "Error inserting the item.\n");
    }
    return NULL;
}

// consumer thread function
void *consumer(void *param) {
    BUFFER_ITEM item;
    while (1) {
        /* release the CPU randomly to simulate preemption */
        if ((rand() % 100) < 40) { // ~40% chance
            sched_yield(); // voluntarily yield CPU
        }

        // try removing item
        if (remove_item(&item))
            fprintf(srderr, "Error removing item.\n");
    
        // verify item matches that inserted based on checksum
        uint16_t new_cksum = calculateChecksum(item.data);
        if (new_cksum != item.cksum) {
            fprintf(stderr, "Checksum doesn't match. The given sum was %u and the calculated sum was %u\n", item.cksum, new_cksum);
            pthread_exit(NULL);
        }
    }
    return NULL;
}

/* insert item into buffer, return O if successful, otherwise return -1 indicating an error condition */
int insert_item(BUFFER_ITEM item) {
    
    // check if semaphore is not empty (return error if so)
    if (sem_wait(&empty) != 0) {
        return -1;
    }

    // check if mutex is not locked
    if (pthread_mutex_lock(&mutex) != 0) {
        sem_post(&empty); // undoes the wait we did
        return -1;
    }

    // insert into buffer at in location
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE; // mod to loop around

    // unlock the buffer
    pthread_mutex_unlock(&mutex);

    // tell semaphore that there is now a newly full spot
    sem_post(&full);

    // return 0 since there were no errors
    return 0;
}

/* remove an object from buffer placing it in item return O if successful, otherwise return -1 indicating an error condition */
int remove_item(BUFFER_ITEM* item) {

    // check if there is at least one full slot
    if (sem_wait(&full) != 0) {
        return -1;
    }

    // check if mutex is locked
    if (pthread_mutex_lock(&mutex) != 0) {
        sem_post(&full);
        return -1;
    }    
    
    // take item out of buffer
    *item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;

    // unlock mutex
    pthread_mutex_unlock(&mutex);

    // tell semaphore there is another empty slot
    sem_post(&empty);
    
    // return 0 since there were no errors
    return 0;
}

int main (int argc, char* argv[]) {
    /* 1. Get command line arguments argv[1], argv[2], argv [3] */
    if(argc != 4) {
        printf("ERROR: Incorrect Usage. Please use %s <SECONDS_DELAY> <NUM_OF_PRODUCER THREADS> <NUM_OF_CONSUMER_THREADS>");
        
        // return because an error has occurred
        return -1;
    }

    /* 2. Initialize buffer */
    buffer 
    
    /* 3. Create producer thread(s) */ 
    /* 4. Create consumer thread(s) */
    /* 5. Sleep */ 
    /* 6. Exit */
}
