## AUTHOR INFO
Full Name: Dylan Ravel and Daniel Tsivkovski

Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu

Course Number and section: CPSC-380-02

Assignment or Exercise Number: Assignment 2 - Thread Synchronization

## ERRORS
We have not observed any errors from our testing.

## SOURCES
Below are all the sources that we used when coding this project.
    - [sem_init](https://man7.org/linux/man-pages/man3/sem_init.3.html)
    - [pthread_mutex_init](https://man7.org/linux/man-pages/man3/pthread_mutex_init.3p.html#top_of_page)
    - [pthread_cancel](https://man7.org/linux/man-pages/man3/pthread_cancel.3.html)

## RUNNING INSTRUCTIONS
Compile the file as so: `gcc prodcon.c checksum.c -pthread -o prodcon`

Run the file as so: `./prodcon <SECONDS_DELAY> <NUM_OF_PRODUCER THREADS> <NUM_OF_CONSUMER_THREADS>`

Example: `./prodcon 10 3 3`
