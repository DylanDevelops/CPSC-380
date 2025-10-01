/** 
 * Full Name: Dylan Ravel and Daniel Tsivkovski
 * Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 2 - Thread Synchronization
*/

#include <stdint.h>

typedef struct buffer_item {
    uint8_t data[30];
    uint16_t cksum;
} BUFFER_ITEM;

#define BUFFER_SIZE 10