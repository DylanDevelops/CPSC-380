/** 
 * Full Name: Daniel Tsivkovski and Dylan Michael Ravel
 * Chapman Email: tsivkovski@chapman.edu, ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 4 - CPU Scheduling Simulation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>
#include <unistd.h>

// define max processes
#define MAX_PROC 100
#define FILE_LINE_SIZE 100
#define MAX_TIMELINE_SIZE 1000

typedef struct {
    // information from csv
    char pid[10];
    int arrival;
    int burst;
    int priority;
    // variables to keep track of process for metrics
    int remaining_time;
    int start_time;
    int finish_time;
    int waiting_time;
    int response_time;
    int turnaround_time;
    int first_run;
    // semaphore and thread for process
    sem_t sem;
    pthread_t thread;
} Process;

typedef enum {
    FCFS,
    SJF,
    PRIORITY,
    RR
} Algorithm; // Algorithm types

typedef struct {
    char pid[10];
    int start;
    int end;
} Event; // Event for Gantt chart

// Global variables for scheduler
Process processes[MAX_PROC];
int num_proc = 0;
int curr_time = 0;
Algorithm sched_algo = FCFS;
int quantum = 2;
sem_t scheduler_sem;

// READY Queue
Process *ready_queue[MAX_PROC];
int rq_size = 0;

// timeline for Gantt chart
Event timeline[MAX_TIMELINE_SIZE];
int event_count = 0;

// function prototypes
void csv_parser(const char* filename);
void* process_thread(void* arg);
void enqueue(Process* p);
Process* dequeue();
Process* choose_next_proc();
void run_scheduler();
void print_gantt();
void print_metrics();

int main(int argc, char* argv[]) {
    char* file_in = NULL;
    int opt;

    // define options
    static struct option long_options[] = {
        {"fcfs", no_argument, 0, 'f'},
        {"sjf", no_argument, 0, 's'},
        {"priority", no_argument, 0, 'p'},
        {"rr", no_argument, 0, 'r'},
        {"quantum", required_argument, 0, 'q'},
        {"input", required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };

    // parse command line arguments
    while ((opt = getopt_long(argc, argv, "fsprq:i:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                sched_algo = FCFS;
                break;
            case 's':
                sched_algo = SJF;
                break;
            case 'p':
                sched_algo = PRIORITY;
                break;
            case 'r':
                sched_algo = RR;
                break;
            case 'q':
                quantum = atoi(optarg);
                break;
            case 'i':
                file_in = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [--fcfs|--sjf|--priority|--rr] [--quantum <num>] --input <file>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // check if missing input
    if (file_in == NULL) {
        fprintf(stderr, "Error: Input file is required for this program to run.\n");
        exit(EXIT_FAILURE);
    }

    // parse CSV file
    csv_parser(file_in);

    // create scheduler semaphore
    sem_init(&scheduler_sem, 0, 0);

    // create and initialize threads
    for (int i = 0; i < num_proc; ++i) {
        sem_init(&processes[i].sem, 0, 0); // init semaphore to 0
        processes[i].remaining_time = processes[i].burst;
        processes[i].first_run = 1; // helps indicate start time
        pthread_create(
            &processes[i].thread, 
            NULL, 
            process_thread, 
            (void*)&processes[i]
        );
    }

    run_scheduler();

    // wait for all threads to finish
    for (int i = 0; i < num_proc; ++i) {
        pthread_join(processes[i].thread, NULL);
        sem_destroy(&processes[i].sem);
    }

    // print gantt chart and metrics
    printf("===== ");
    switch (sched_algo) {
        case FCFS:
            printf("FCFS");
            break;
        case SJF:
            printf("SJF");
            break;
        case PRIORITY:
            printf("PRIORITY");
            break;
        case RR:
            printf("RR WITH QUANTUM = %d", quantum);
            break;
    }
    printf(" SCHEDULING =====\n");
    print_gantt();
    printf("-----------------\n");
    printf("-----------------\n");
    print_metrics();
    return 0;
}

// function to parse CSV file into processes
void csv_parser(const char* filename) {
    FILE* file = fopen(filename, "r"); // open file with readonly
    if (!file) {
        // throw error and exit
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[FILE_LINE_SIZE];

    // skip the header line
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Error reading header from file\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // parse each process into program
    while (fgets(line, sizeof(line), file) && num_proc < MAX_PROC) {
        
        // parse line into process fields for process at index
        sscanf(
            line, 
            "%[^,],%d,%d,%d", 
            processes[num_proc].pid, 
            &processes[num_proc].arrival, 
            &processes[num_proc].burst, 
            &processes[num_proc].priority
        );

        num_proc++;
    }

    // close file when done
    fclose(file);
}

// thread function for each process
void* process_thread(void* arg) {
    Process *p = (Process*)arg; // cast arg to process

    // spin until process is complete
    while (p->remaining_time > 0) {
        // wait for sched to dispatch
        sem_wait(&p->sem);

        // record start time if first run
        if (p->first_run) {
            p->start_time = curr_time;
            p->first_run = 0;
        }

        // remove one unit of remaining time
        p->remaining_time--;

        // tell scheduler that process is done with one time cycle
        sem_post(&scheduler_sem);

    }

    return NULL;
}

// standard enqueue function for a queue
void enqueue(Process* p) {
    // add process to end of queue and iterate
    ready_queue[rq_size++] = p; 
}

// standard dequeu function for a queue
Process* dequeue() {
    if (rq_size == 0) return NULL; // if empty return null
    
    // get process at the front
    Process* p = ready_queue[0];

    // shift all processes forward
    for (int i = 1; i < rq_size; ++i) {
        ready_queue[i - 1] = ready_queue[i];
    }

    // decrement size and return
    rq_size--;
    return p;
}

Process* choose_next_proc() {
    switch(sched_algo) {
        case FCFS:
            return dequeue(); // FCFS gets first process
            break;
        case SJF: {
            // find process with shortest remaining time
            int min_idx = -1;
            int min_time = __INT_MAX__;
            for (int i = 0; i < rq_size; ++i) {
                if (ready_queue[i]->remaining_time < min_time) {
                    min_time = ready_queue[i]->remaining_time;
                    min_idx = i;
                }
            }
            // return null if none found or return process at idx
            if (min_idx == -1) return NULL;
            Process* p = ready_queue[min_idx];
            // shift remaining processes down one
            for (int i = min_idx + 1; i < rq_size; ++i) {
                ready_queue[i - 1] = ready_queue[i];
            }
            // update size and return
            rq_size--;
            return p;
            break;
        }
        case PRIORITY: {
            // get process with the highest priority
            int chosen_idx = -1;
            int highest_priority = __INT_MAX__;
            for (int i = 0; i < rq_size; ++i) {
                if (ready_queue[i]->priority < highest_priority) {
                    highest_priority = ready_queue[i]->priority;
                    chosen_idx = i;
                }
            }
            // return null if none found or process at idx
            if (chosen_idx == -1) return NULL;
            Process* p = ready_queue[chosen_idx];
            // shift remaining processes down one
            for (int i = chosen_idx + 1; i < rq_size; ++i) {
                ready_queue[i - 1] = ready_queue[i];
            }
            // update size and return
            rq_size--;
            return p;
            break;
        }
        case RR:
            return dequeue(); // RR gets first process
            break;
        default:
            return NULL;
    }
}

void run_scheduler() {
    int completed_proc = 0;
    int t_slice = quantum; // track remaining time slice of quantum for RR
    Process* curr_proc = NULL; // track the currently running process
    Process* last_run_proc = NULL;
    int event_start = 0;

    while (completed_proc < num_proc) {
        // check any arriving processes
        for (int i = 0; i < num_proc; ++i) {
            if (processes[i].arrival == curr_time) {
                enqueue(&processes[i]); // standard enqueue for all algorithms

                // preemption check for priority on new arrival
                if (sched_algo == PRIORITY && curr_proc != NULL) {
                    if (processes[i].priority < curr_proc->priority) {
                        enqueue(curr_proc);
                        curr_proc = NULL;
                    }
                }
            }
        }

        // choose next process with quantum check
        Process* next_proc = NULL;
        if (curr_proc == NULL) {
            next_proc = choose_next_proc();
            if (next_proc != NULL) {
                curr_proc = next_proc;
                t_slice = quantum; // reset time slice for RR
            }
        } else {
            next_proc = curr_proc;
        }

        if (next_proc != NULL) {

            // if process has changed save previous event
            if (last_run_proc != NULL && last_run_proc != next_proc) {
                // save to timeline
                strcpy(timeline[event_count].pid, last_run_proc->pid);
                timeline[event_count].start = event_start;
                timeline[event_count].end = curr_time;
                event_count++;
                // update event start time
                event_start = curr_time;
            }

            // if first process then set event start
            if (last_run_proc == NULL) {
                event_start = curr_time;
            }

            // update last run
            last_run_proc = next_proc;

            // dispatch process to semaphore
            sem_post(&next_proc->sem);
            // wait for process to complete cycle
            sem_wait (&scheduler_sem);
            
            // decrement time slice for RR
            if (sched_algo == RR) {
                t_slice--;
            }

            // check if process is complete 
            if (next_proc->remaining_time == 0) {
                next_proc->finish_time = curr_time + 1;
                completed_proc++;
                curr_proc = NULL;
            } else if (sched_algo == RR && t_slice == 0) {
                // if time slice expired for RR then we must requeue
                enqueue(next_proc);
                curr_proc = NULL;
            } else {
                curr_proc = next_proc; // continue with current process for other algorithms
            }
        } 

        // iterate time
        curr_time++;
    }

    // save final event to timeline
    if (last_run_proc != NULL) {
        strcpy(timeline[event_count].pid, last_run_proc->pid);
        timeline[event_count].start = event_start;
        timeline[event_count].end = curr_time;
        event_count++;
    }
}

void print_gantt() {
    printf("\nTimeline (Gantt Chart):\n");
    
    // time marker printing
    for (int i = 0; i < event_count; i++) {
        printf("%-6d", timeline[i].start);
    }
    printf("%d\n", timeline[event_count-1].end);
    
    // borders
    for (int i = 0; i < event_count; i++) printf("|-----");
    printf("|\n");
    
    // process IDs
    for (int i = 0; i < event_count; i++) printf("| %-3s ", timeline[i].pid);
    printf("|\n");
    
    // bottom borders
    for (int i = 0; i < event_count; i++) printf("|-----");
    printf("|\n");
}

void print_metrics() {
    // header
    printf("PID\tArrival\tBurst\tFinish\tWaiting\tTurnaround\tResponse\n");
    // iterate through all processes to calculate then print
    for (int i = 0; i < num_proc; ++i) {

        // calculate remaininng metrics
        processes[i].turnaround_time = processes[i].finish_time - processes[i].arrival;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst;
        processes[i].response_time = processes[i].start_time - processes[i].arrival;

        // print all of the metrics for process
        printf("%s\t%d\t%d\t%d\t%d\t%d\t\t%d\n",
            processes[i].pid,
            processes[i].arrival,
            processes[i].burst,
            processes[i].finish_time,
            processes[i].waiting_time,
            processes[i].turnaround_time,
            processes[i].response_time
        );
    }

    // calculate averages
    double avg_wait = 0, avg_turnaround = 0, avg_response = 0;
    for (int i = 0; i < num_proc; ++i) {
        avg_wait += processes[i].waiting_time;
        avg_turnaround += processes[i].turnaround_time;
        avg_response += processes[i].response_time;
    }
    avg_wait /= num_proc;
    avg_turnaround /= num_proc;
    avg_response /= num_proc;

    int total_time = curr_time;
    float throughput = (float)num_proc / (float)total_time; 
    float cpu_util = 100.0; // no idle time in this simulation

    // print averages
    printf("-----------------\n");
    printf("\nAverage Wait Time: %.2f\n", avg_wait);
    printf("Average Turnaround Time: %.2f\n", avg_turnaround);
    printf("Average Response Time: %.2f\n", avg_response);
    printf("Throughput: %.2f jobs/unit time\n", throughput);
    printf("CPU Utilization: %.2f%%\n", cpu_util);
}