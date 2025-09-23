/** 
 * Full Name: Dylan Ravel
 * Chapman Email: ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: InClass - Threads
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

double average;
int min, max;
int count;

int numbers[20];

void* computeAverage(void* arg) {
    int sum = 0;
    
    for(int i = 0; i < count; ++i) {
        sum += numbers[i];
    }

    average = (double)sum / (double)count;

    return NULL;
}

void* computeMax(void* arg) {
    max = numbers[0];
    
    for(int i = 0; i < count; ++i) {
        if(numbers[i] > max) {
            max = numbers[i];
        }
    }

    return NULL;
}

void* computeMin(void* arg) {
    min = numbers[0];
    
    for(int i = 0; i < count; ++i) {
        if(numbers[i] < min) {
            min = numbers[i];
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("Usage: %s <int array> \n ", argv[0]);
        return -1;
    }

    count = argc - 1;

    for(int i = 1; i < argc; ++i) {
        numbers[i-1] = atoi(argv[i]);

    }

    pthread_t threadId[3];

    pthread_create(&threadId[0], NULL, computeAverage, NULL);
    pthread_create(&threadId[1], NULL, computeMax, NULL);
    pthread_create(&threadId[2], NULL, computeMin, NULL);

    pthread_join(threadId[0], NULL);
    pthread_join(threadId[1], NULL);
    pthread_join(threadId[2], NULL);

    printf("Average: %lf \n", average);
    printf("Maximum: %d \n", max);
    printf("Minimum: %d \n", min);

    return 0;
}