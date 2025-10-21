/** 
 * Full Name: Dylan Ravel and Daniel Tsivkovski
 * Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 3 - Reader-Writer Syncronization
*/

/* Add appropriate header files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include "rw_log.h"

struct config {
    int capacity;
    int readers;
    int writers;
    int writer_batch;
    int seconds;
    int rd_us;
    int wr_us;
    int dump_csv;
};

typedef struct {
    // mutex and condition variables for monitor
    pthread_mutex_t lock;
    pthread_cond_t reader_cond;
    pthread_cond_t writer_cond;

    // variables for sync checking
    int active_readers;
    int active_writers;
    int waiting_writers;

    // monitor log variables
    size_t capacity; // max number of entries
    size_t write_index; // next write index
    size_t read_index; // next read index

    // entry management
    size_t count; // current number of entries
    u_int64_t entry_seq; // unique entry sequence that increases
    rwlog_entry_t entries[];
} rwlog_monitor_t;

typedef struct {
    // variables for thread
    int id;
    int batch_size;
    int sleep_us;

    // for stats
    double total_wait_time;
    int num_writes;
} writer_data_t;

typedef struct {
    // variables for thread 
    int id;
    int sleep_us;

    // for stats
    double total_read_time;
    int num_reads;
} reader_data_t;

// Globals
static rwlog_monitor_t* monitor = NULL;
static volatile sig_atomic_t stop_flag = 0; // use sig atomic t for safety as it can be changed in signal handler atomically

static void print_usage(const char *progname) 
{
	fprintf(stderr,
        "Usage: %s [options]\n"
        "Options:\n"
        "-c,  --capacity <N>        Log capacity (default 1024)\n"
        "-r,  --readers <N>         Number of reader threads (default 6)\n"
        "-w,  --writers <N>         Number of writer threads (default 4)\n"
        "-b,  --writer-batch <N>    Entries written per writer section (default 2)\n"
        "-s,  --seconds <N>         Total run time (default 10)\n"
        "-R,  --rd-us <usec>        Reader sleep between operations (default 2000)\n"
        "-W,  --wr-us <usec>        Writer sleep between operations (default 3000)\n"
        "-d,  --dump                Dump final log to log.csv\n"
        "-h,  --help                Show this help message\n",
        progname);
}


// function that checks if args are invalid
static int check_args(struct config* cfg) {
    // check arg requirements
    int valid = 1;

    if (cfg->capacity <= 0) {
        fprintf(stderr, "Error: capacity must be greater than 0.\n");
        valid = 0;
    }

    if (cfg->readers <= 0) {
        fprintf(stderr, "Error: number of readers must be greater than 0.\n");
        valid = 0;
    }

    if (cfg->writers <= 0) {
        fprintf(stderr, "Error: number of writers must be greater than 0.\n");
        valid = 0;
    }

    if (cfg->writer_batch <= 0) {
        fprintf(stderr, "Error: writer batch size must be greater than 0.\n");
        valid = 0;
    }

    if (cfg->seconds <= 0) {
        fprintf(stderr, "Error: total run time must be greater than 0.\n");
        valid = 0;
    }

    if (cfg->rd_us < 0) {
        fprintf(stderr, "Error: reader sleep time must be non-negative.\n");
        valid = 0;
    }

    if (cfg->wr_us < 0) {
        fprintf(stderr, "Error: writer sleep time must be non-negative.\n");
        valid = 0;
    }
    
    return valid;
}

static void parse_args(int argc, char **argv, struct config *cfg) 
{
	// Set defaults
    cfg->capacity     = 1024;
    cfg->readers      = 6;
    cfg->writers      = 4;
    cfg->writer_batch = 2;
    cfg->seconds      = 10;
    cfg->rd_us        = 2000;
    cfg->wr_us        = 3000;
    cfg->dump_csv     = 0;

    int option; // option for getopt_long
	
	/* define the long_opts options */
	static struct option long_options[] = {
        {"capacity", required_argument, 0, 'c'},
        {"readers", required_argument, 0, 'r'},
        {"writers", required_argument, 0, 'w'},
        {"writer-batch", required_argument, 0, 'b'},
        {"seconds", required_argument, 0, 's'},
        {"rd-us", required_argument, 0, 'R'},
        {"wr-us", required_argument, 0, 'W'},
        {"dump", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };
	
	/* Parse each of the arguments using getopt_long() function */
    while ((option = getopt_long(argc, argv, "c:r:w:b:s:R:W:dh", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                cfg->capacity = atoi(optarg);
                break;

            case 'r':
                cfg->readers = atoi(optarg);
                break;
                
            case 'w':
                cfg->writers = atoi(optarg);
                break;

            case 'b':
                cfg->writer_batch = atoi(optarg);
                break;

            case 's':
                cfg->seconds = atoi(optarg);
                break;

            case 'R':
                cfg->rd_us = atoi(optarg);
                break;

            case 'W':
                cfg->wr_us = atoi(optarg);
                break;

            case 'd':
                cfg->dump_csv = 1;
                break;
            
            case 'h':
                print_usage(argv[0]); // prints usage with program name
                exit(0);
                break;
            default:
                abort();
                break;
        }
    }

    // run function to check for invalid args
    if (!check_args(cfg)) {
        print_usage(argv[0]);
        exit(1);
    }
	
}

// creates the rwlog monitor in shared memory
int rwlog_create(size_t capacity) {
    // create monitor in shared memory with space for entries
    monitor = (rwlog_monitor_t *) malloc(sizeof(rwlog_monitor_t) + capacity * sizeof(rwlog_entry_t));
    
    // check if malloc didn't work
    if(!monitor) {
        return -1;
    }

    // initialize monitor variables
    monitor->capacity = capacity;
    monitor->count = 0;
    monitor->active_readers = 0;
    monitor->active_writers = 0;
    monitor->waiting_writers = 0;
    monitor->entry_seq = 0;
    monitor->write_index = 0;
    monitor->read_index = 0;

    // initialize pthread mutex safely
    if (pthread_mutex_init(&monitor->lock, NULL) != 0) {
        // throw error and clean up if failed
        perror("pthread_mutex_init");
        free(monitor);
        monitor = NULL;
        return -1;
    }

    // initialize reader condition variable safely
    if (pthread_cond_init(&monitor->reader_cond, NULL) != 0) {
        // same as above, clean up and throw error if failed
        perror("pthread_cond_init (reader)");
        pthread_mutex_destroy(&monitor->lock);
        free(monitor);
        monitor = NULL;
        return -1;
    }

    // initialize writer condition variable safely
    if (pthread_cond_init(&monitor->writer_cond, NULL) != 0) {
        // same as above, clean up and throw error if failed
        perror("pthread_cond_init (writer)");
        pthread_cond_destroy(&monitor->reader_cond);
        pthread_mutex_destroy(&monitor->lock);
        free(monitor);
        monitor = NULL;
        return -1;
    }

    return 0;
}

void rwlog_wake_all() {
    if (!monitor) return;
    // lock mutex to safely wake all threads
    pthread_mutex_lock(&monitor->lock);
    // wake up all threads waiting on condition variables
    pthread_cond_broadcast(&monitor->reader_cond);
    pthread_cond_broadcast(&monitor->writer_cond);
    // unlock mutex after waking
    pthread_mutex_unlock(&monitor->lock);
}

int rwlog_destroy(void) {
    if(!monitor) return -1;
    
    // destroy each syncronization thread
    pthread_cond_destroy(&monitor->writer_cond);
    pthread_cond_destroy(&monitor->reader_cond);
    pthread_mutex_destroy(&monitor->lock);

    // clean up memory
    free(monitor);
    monitor = NULL;
    
    return 0;
}

static void handle_sigint(int sig) {
    (void)sig; // sig is unused
    stop_flag = 1;
    rwlog_wake_all();
}

static void* thread_timer(void* arg) {
    // sleep for this amount of seconds
    int seconds = *(int*)arg;

    // check for stop flag every half second
    for (int i = 0; i < seconds * 2; ++i) {
        if (stop_flag) {
            break; 
        }
        usleep(500000); // 500ms sleep
    }
    
    // set stop flag and wake all threads
    stop_flag = 1;
    rwlog_wake_all();

    return NULL;
}

int rwlog_begin_write(void) {
    if(!monitor) return -1;

    // lock mutex
    pthread_mutex_lock(&monitor->lock);
    // update number of waiting writers
    monitor->waiting_writers++;

    // check if any active readers or writers
    while(monitor->active_readers > 0 || monitor->active_writers > 0) {
        // wait on writer condition variable
        pthread_cond_wait(&monitor->writer_cond, &monitor->lock);

        if (stop_flag) { // check if stop flag to exit if needed
            monitor->waiting_writers--;
            pthread_mutex_unlock(&monitor->lock);
            return -1;
        }
    }
    
    // update monitor states (removing one from waiting because it's now active)
    monitor->waiting_writers--;
    monitor->active_writers = 1;
    pthread_mutex_unlock(&monitor->lock); // unlock

    return 0;
}

int rwlog_end_write(void) {
    if (!monitor) return -1;

    pthread_mutex_lock(&monitor->lock);

    monitor->active_writers = 0;

    // if there are writers waiting, start one writer to avoid starvation
    if(monitor->waiting_writers > 0) {
        pthread_cond_signal(&monitor->writer_cond);
    } else { // otherwise, wake all readers
        pthread_cond_broadcast(&monitor->reader_cond);
    }

    // finally release the lock after all the modifications
    pthread_mutex_unlock(&monitor->lock);
    
    return 0;
}

// appends an entry to the log
int rwlog_append(const rwlog_entry_t* e) {
    if (!monitor || !e) {
        fprintf(stderr, "Error: monitor not initialized or entry is NULL.\n");
        return -1;
    }

    // lock mutex to safely append
    pthread_mutex_lock(&monitor->lock);

    // get position to write
    size_t pos = monitor->write_index;
    monitor->entries[pos] = *e; // copy entry to log
    monitor->entries[pos].seq = monitor->entry_seq++; // give it unique sequence number
    monitor->entries[pos].tid = pthread_self(); // set thread id
    monitor->write_index = (pos + 1) % monitor->capacity; // update write index
    clock_gettime(CLOCK_REALTIME, &monitor->entries[pos].ts); // timestamp entry

    // update count if not full
    if (monitor->count < monitor->capacity) {
        monitor->count++;
    } else {
        // if full, move read index forward to overwrite oldest
        monitor->read_index = (monitor->read_index + 1) % monitor->capacity;
    }

    // unlock mutex when done
    pthread_mutex_unlock(&monitor->lock);
    return 0;
}

static void* write_thread(void* arg) {
    writer_data_t* data = (writer_data_t*)arg;
    struct timespec start, end;

    while (!stop_flag) {
        // start timer
        clock_gettime(CLOCK_MONOTONIC, &start);

        // try to write
        if (rwlog_begin_write() != 0) {
            break; // exit if failed
        }

        // end timer and find wait time
        clock_gettime(CLOCK_MONOTONIC, &end);
        double wait_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        data->total_wait_time += wait_time;

        // write batch size entries
        for (int i = 0; i < data->batch_size; ++i) {
            // create entry and write it
            rwlog_entry_t entry;
            snprintf(entry.msg, RWLOG_MSG_MAX, "Writer %d: Entry %d", data->id, data->num_writes + i);
            
            // try to append entry
            if (rwlog_append(&entry) != 0) {
                break;
            }
        }
        data->num_writes += data->batch_size; // update num writes after batch writes

        // end write
        rwlog_end_write();

        // sleep for specified time
        if (!stop_flag && data->sleep_us > 0) {
            usleep(data->sleep_us);
        }
    }

    return NULL;
}

int rwlog_begin_read(void) {
    if(!monitor) return -1;

    // lock mutex
    pthread_mutex_lock(&monitor->lock);

    // while there is an active writer or multiple waiting, wait.
    while((monitor->active_writers > 0 || monitor->waiting_writers > 0)) {
        pthread_cond_wait(&monitor->reader_cond, &monitor->lock);

        if(stop_flag) { // exit if there is a stop flag
            pthread_mutex_unlock(&monitor->lock);
            return -1;
        }
    }

    // update active readers count
    monitor->active_readers++;
    pthread_mutex_unlock(&monitor->lock); // unlock
    
    return 0;
}

int rwlog_end_read(void) {
    if(!monitor) return -1;

    // lock mutex and decrement number of active readers
    pthread_mutex_lock(&monitor->lock);
    monitor->active_readers--;

    // if no readers remain, and there are writers waiting, start one to avoid starvation
    if(monitor->active_readers == 0 && monitor->waiting_writers > 0) {
        pthread_cond_signal(&monitor->writer_cond);
    }

    // unlock mutex
    pthread_mutex_unlock(&monitor->lock); // unlock

    return 0;
}

ssize_t rwlog_snapshot(rwlog_entry_t *buf, size_t max_entries) {
    // check if no monitor or buffer
    if (!monitor || !buf) {
        return -1;
    }
    
    // lock mutex
    pthread_mutex_lock(&monitor->lock);

    // how many entries to copy
    size_t num_copy = (monitor->count < max_entries) ? monitor->count : max_entries;
    
    if (num_copy > 0) {
        for (size_t i = 0; i < num_copy; ++i) {
            // get index and copy entry to buffer at index
            size_t index = (monitor->read_index + i) % monitor->capacity;
            buf[i] = monitor->entries[index];
        }
    }

    // unlock mutex
    pthread_mutex_unlock(&monitor->lock);

    // return num entries copied
    return (ssize_t)num_copy;
    
}

static void* read_thread(void* arg) {
    // variables for thread
    reader_data_t* data = (reader_data_t*)arg;
    struct timespec start, end;

    // snapshot buffer
    rwlog_entry_t* buf = malloc(monitor->capacity * sizeof(rwlog_entry_t));
    // check successful allocation
    if (!buf) {
        fprintf(stderr, "Error allocating for buffer\n");
        return NULL;
    }

    while(!stop_flag) {
        // start timer
        clock_gettime(CLOCK_MONOTONIC, &start);

        // try to begin read
        if (rwlog_begin_read() != 0) {
            break; // exit if failed
        }

        // snapshot log
        ssize_t count = rwlog_snapshot(buf, monitor->capacity);

        // monotonicity check
        for (ssize_t i = 1; i < count; ++i) {
            if (buf[i].seq < buf[i-1].seq) {
                fprintf(stderr, "Error: Reader %d found non-monotonic sequence number %llu after %llu\n",
                        data->id, (unsigned long long)buf[i].seq, (unsigned long long)buf[i-1].seq);
            }
        }

        // end read
        rwlog_end_read();

        // end timer and calculate read time
        clock_gettime(CLOCK_MONOTONIC, &end);
        double read_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        data->total_read_time += read_time;
        data->num_reads++; // iterate num reads

        // sleep for specified time
        if (!stop_flag && data->sleep_us > 0) {
            usleep(data->sleep_us);
        }

    }

    free(buf);
    return NULL;
}

// dumps the log to a csv file
static void dump_log_to_csv() {
    if (!monitor) return;

    FILE* file = fopen("log.csv", "w");

    if (!file) {
        fprintf(stderr, "Error opening or creating log.csv for writing\n");
        return;
    } else {
        // create csv header
        fprintf(file, "seq,tid,timestamp,msg\n");

        // begin read
        if (rwlog_begin_read() != 0) {
            fprintf(stderr, "Error beginning read for CSV dump\n");
            fclose(file);
            return;
        }

        // create buffer for one snapshot
        rwlog_entry_t* buf = malloc(monitor->capacity * sizeof(rwlog_entry_t));
        // check successful allocation
        if (!buf) {
            fprintf(stderr, "Error allocating memory for snapshot buffer\n");
            fclose(file);
            return;
        }

        // take snapshot
        ssize_t count = rwlog_snapshot(buf, monitor->capacity);
        if (count < 0) {
            fprintf(stderr, "Error taking a snapshot of the log\n");
            free(buf);
            fclose(file);
            return;
        }

        // write each entry
        for (ssize_t i = 0; i < count; ++i) {
            fprintf(file, "%llu,%lu,%llu.%09lu,%s\n",
                (unsigned long long)buf[i].seq,
                (unsigned long)buf[i].tid,
                (unsigned long long)buf[i].ts.tv_sec,
                (unsigned long)buf[i].ts.tv_nsec,
                buf[i].msg);
        }
        free(buf); // free buffer when done

        // end read
        rwlog_end_read();

    }

    // close file when done
    fclose(file);
}

int main(int argc, char **argv) 
{
    struct config cfg;
    parse_args(argc, argv, &cfg);

    printf("capacity=%d readers=%d writers=%d batch=%d seconds=%d rd_us=%d wr_us=%d dump=%d\n",
           cfg.capacity, cfg.readers, cfg.writers, cfg.writer_batch,
           cfg.seconds, cfg.rd_us, cfg.wr_us, cfg.dump_csv);

    /* your remaining initialization here... */	
	
	/* Initialize the shm-backed monitor */
    if (rwlog_create((size_t)cfg.capacity) != 0) {
        fprintf(stderr, "Failed to create log monitor\n");
        return 1;
    }
 
    /* Install SIGINT and start wall-clock timer thread */
	signal(SIGINT, handle_sigint);

    // create timer thread and check for error
    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, thread_timer, &cfg.seconds) != 0) {
        fprintf(stderr, "Error creating timer thread\n");
        rwlog_destroy();
        return 1;
    }
	
    /* Create the writer threads */
    pthread_t* writerThreads = malloc(cfg.writers * sizeof(pthread_t));
    // check successful allocation
    if (!writerThreads) {
        fprintf(stderr, "Error allocating memory for writer threads\n");
        rwlog_destroy();
        return 1;
    }
    writer_data_t* writer_data = calloc(cfg.writers, sizeof(writer_data_t));
    // check successful allocation
    if (!writer_data) {
        fprintf(stderr, "Error allocating memory for writer data\n");
        free(writerThreads);
        rwlog_destroy();
        return 1;
    }

    // iterate through all to create pthreads
    for (int i = 0; i < cfg.writers; ++i) {
        writer_data[i].id = i;
        writer_data[i].batch_size = cfg.writer_batch;
        writer_data[i].sleep_us = cfg.wr_us;
        writer_data[i].total_wait_time = 0.0;
        writer_data[i].num_writes = 0;

        // create pthread with error checking
        if (pthread_create(&writerThreads[i], NULL, write_thread, &writer_data[i]) != 0) {
            fprintf(stderr, "Error creating writer thread %d\n", i);
            rwlog_destroy();
            return 1;
        }
    }
	 
    /* Create the reader threads */
    pthread_t* readerThreads = malloc(cfg.readers * sizeof(pthread_t));
    // check successful allocation
    if (!readerThreads) {
        fprintf(stderr, "Error allocating memory for reader threads\n");
        rwlog_destroy();
        return 1;
    }
    reader_data_t* reader_data = calloc(cfg.readers, sizeof(reader_data_t));
    // check successful allocation
    if (!reader_data) {
        fprintf(stderr, "Error allocating memory for reader data\n");
        free(readerThreads);
        rwlog_destroy();
        return 1;
    }


    // iterate through all to create pthreads
    for (int i = 0; i < cfg.readers; ++i) {
        reader_data[i].id = i;
        reader_data[i].sleep_us = cfg.rd_us;
        reader_data[i].total_read_time = 0.0;
        reader_data[i].num_reads = 0;

        // create reader thread and check for error
        if (pthread_create(&readerThreads[i], NULL, read_thread, &reader_data[i]) != 0) {
            fprintf(stderr, "Error creating reader thread %d\n", i);
            rwlog_destroy();
            return 1;
        }
    }
	 
    /* Join reader/writer threads and timer thread */
    pthread_join(timer_thread, NULL);
    // iterate and join all writer threads
    for (int i = 0; i < cfg.writers; ++i) {
        pthread_join(writerThreads[i], NULL);
    }

    // iterate and join all reader threads
    for (int i = 0; i < cfg.readers; ++i) {
        pthread_join(readerThreads[i], NULL);
    }
	 
    /* Optional: dump the final log to CSV for inspection/grading. */
    if (cfg.dump_csv) {
        dump_log_to_csv();
    }
	 
    /* Compute averages only (avg reader wait, avg writer wait, avg throughput) */
    // get total writer stats
    double total_writer_wait = 0.0;
    int total_writes = 0;
    for (int i = 0; i < cfg.writers; ++i) {
        total_writer_wait += writer_data[i].total_wait_time;
        total_writes += writer_data[i].num_writes;
    }

    // get total reader stats
    double total_reader_time = 0.0;
    int total_reads = 0;
    for (int i = 0; i < cfg.readers; ++i) {
        total_reader_time += reader_data[i].total_read_time;
        total_reads += reader_data[i].num_reads;
    }

    // print averages
    double avg_writer_wait_ms = (total_writes > 0) ? (total_writer_wait / total_writes) * 1000.0 : 0.0;
    double avg_reader_time_ms = (total_reads > 0) ? (total_reader_time / total_reads) * 1000.0 : 0.0;
    double throughput = (double)total_writes / (double)cfg.seconds;

    // print stats
    fprintf(stdout, "--- Stats ---\n");
    fprintf(stdout, "Average writer wait time: %.3fms\n", avg_writer_wait_ms);
    fprintf(stdout, "Average reader critical-section time: %.3fms\n", avg_reader_time_ms);
    fprintf(stdout, "Total log entries written: %d\n", total_writes);
    fprintf(stdout, "Throughput: %.2f entries/sec\n", throughput);

    /* Cleanup heap and monitor resources */
    free(writer_data);
    free(reader_data);
    free(writerThreads);
    free(readerThreads);
    rwlog_destroy();
	  
    return 0;
}