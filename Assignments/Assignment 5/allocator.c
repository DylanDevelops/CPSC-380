/** 
 * Full Name: Daniel Tsivkovski and Dylan Ravel
 * Chapman Email: tsivkovski@chapman.edu, ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 5 - Continuous Memory Allocation
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

typedef struct MemNode {
    char* process;
    int start;
    int end;
    struct MemNode *next;
} MemNode;

MemNode* head = NULL;
int MAX = 0;

MemNode* createNode(char* process, int start, int end) {
    MemNode* n = (MemNode*)malloc(sizeof(MemNode));
    if (n == NULL) {
        fprintf(stderr, "Failed to allocate memory for Node\n");
        exit(EXIT_FAILURE);
    }
    n->process = process;
    n->start = start;
    n->end = end;
    n->next = NULL;
    return n;
}

int insertUsingFirstFit(char* process, int size) {
    // if nothing in the list
    if (head == NULL) {
        // calculate start and end
        int start = 0;
        int end = size - 1;
        
        // Set the head node to a newly created node with the calculated start and end
        head = createNode(process, start, end);
        
        return 0;
    }

    // check if there is a gap before head
    if (head->start >= size) {
        MemNode* temp = head;
        head = createNode(process, 0, size - 1);
        head->next = temp;
        return 0;
    }

    // else iterate through to find good fit
    MemNode* curr = head;
    do {
        // check if next is empty, means we found a fit at the end
        if (curr->next == NULL) {
            int start = curr->end + 1;
            int end = start + size - 1;
            
            // if the end location is outside of bounds
            if (end >= MAX) return -1;

            // create new node as next
            curr->next = createNode(process, start, end);
            return 0;
        } else {
            // check if there's a gap between curr and next process
            if((curr->next->start - curr->end - 1) >= size) {
                // then we can fit process
                int start = curr->end + 1;
                int end = start + size - 1;
                
                // save next node as temporary node, so we can place our new one in between
                MemNode* temp = curr->next;
                curr->next = createNode(process, start, end);
                curr->next->next = temp;
                return 0;
            }
        }

        // advance curr
        curr = curr->next;

    } while (curr->next != NULL);

    return -1; // no adequately sized hole found because our "process" is too big
}

int insertUsingBestFit(char* process, int size) {
    int min_size = MAX;
    int best_start = -1;
    int best_end = -1;
    MemNode* candidate = NULL;

    // if nothing in the list
    if (head == NULL) {
        // calculate start and end
        int start = 0;
        int end = size - 1;
        
        // Set the head node to a newly created node with the calculated start and end
        head = createNode(process, start, end);
        
        return 0;
    }

    // check if there is a gap before head AND it is better than current min size
    if (head->start >= size && head->start < min_size) {
        // update variables for best slot so far
        // candidate remains NULL
        min_size = head->start;
        best_start = 0;
        best_end = size - 1;
    }

    // else iterate through to find good fit
    MemNode* curr = head;
    do {
        int hole = 0;

        // check if next is empty, means we found a hole at the end
        if (curr->next == NULL) {
            // find hole size
            hole = MAX - curr->end - 1;
        } else {
            // check if there's a gap between curr and next process
            hole = curr->next->start - curr->end - 1;
        }

        // check hole size conditions and update candidate
        if (hole < min_size && hole >= size) {
            candidate = curr;
            min_size = hole;
            best_start = curr->end + 1;
            best_end = best_start + size - 1;
        }

        // advance curr
        curr = curr->next;

    } while (curr != NULL);

    if (best_start == -1 || best_end == -1) {
        return -1; // no adequately sized hole found because our "process" is too big
    }

    // now use best candidate
    if (candidate == NULL) {
        MemNode* temp = head;
        head = createNode(process, 0, size - 1);
        head->next = temp;
    } else {
        // save next node as temporary node, so we can place our new one in between (or at the end)
        MemNode* temp = candidate->next;
        candidate->next = createNode(process, best_start, best_end);
        candidate->next->next = temp;
    }

    return 0;
}

int insertUsingWorstFit(char* process, int size) {
    int max_size = 0;
    int best_start = -1;
    int best_end = -1;
    MemNode* candidate = NULL;

    // if nothing in the list
    if (head == NULL) {
        // calculate start and end
        int start = 0;
        int end = size - 1;
        
        // Set the head node to a newly created node with the calculated start and end
        head = createNode(process, start, end);
        
        return 0;
    }

    // check if there is a gap before head AND it is better than current max size
    if (head->start >= size && head->start > max_size) {
        // update variables for best slot so far
        // candidate remains NULL
        max_size = head->start;
        best_start = 0;
        best_end = size - 1;
    }

    // else iterate through to find good fit
    MemNode* curr = head;
    do {
        int hole = 0;

        // check if next is empty, means we found a hole at the end
        if (curr->next == NULL) {
            // find hole size
            hole = MAX - curr->end - 1;
        } else {
            // check if there's a gap between curr and next process
            hole = curr->next->start - curr->end - 1;
        }

        // check hole size conditions and update candidate
        if (hole > max_size && hole >= size) {
            candidate = curr;
            max_size = hole;
            best_start = curr->end + 1;
            best_end = best_start + size - 1;
        }

        // advance curr
        curr = curr->next;

    } while (curr != NULL);

    if (best_start == -1 || best_end == -1) {
        return -1; // no adequately sized hole found because our "process" is too big
    }

    // now use best candidate
    if (candidate == NULL) {
        MemNode* temp = head;
        head = createNode(process, 0, size - 1);
        head->next = temp;
    } else {
        // save next node as temporary node, so we can place our new one in between (or at the end)
        MemNode* temp = candidate->next;
        candidate->next = createNode(process, best_start, best_end);
        candidate->next->next = temp;
    }

    return 0;
}

void request_memory(int argc, char* argv[])
{
    if (argc != 4) { // has to have 4 args (including the 'RQ')
        printf("Invalid number of arguments. Usage: RQ <process> <size> <F|B|W>\n");
        return;
    }
    
    // collect arguments
    char* process = argv[1]; // e.g. P0
    int size = atoi(argv[2]); // e.g. 40000
    char* strategy = argv[3]; // e.g. F | B | W

    // check that a valid size is given
    if(size <= 0) {
        printf("[error] Invalid Size. Please try again.\n");
        return;
    }

    // handle different cases for strategy
    int success = 0;
    if (strcmp(strategy, "F") == 0) { // first fit strategy
        success = insertUsingFirstFit(process, size);
    } else if (strcmp(strategy, "B") == 0) { // best fit strategy
        success = insertUsingBestFit(process, size);
    } else if (strcmp(strategy, "W") == 0) { // worst fit strategy
        success = insertUsingWorstFit(process, size);
    } else {
        // if valid strategy is not passed in, throw error
        printf("[error] Invalid strategy. Please try again.\n");
        return;
    }

    if(success == -1) {
        printf("[error] Insufficent memory exists to fulfill request.\n");
        return;
    } else {
        printf("[success] Allocated %d size to Process %s using %s strategy.\n", size, process, strategy);
    }
}

void release_memory(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Invalid number of arguments. Usage: RL <process>\n");
    }
    printf("Release Memory\n");
}

void compact()
{
    printf("Compact.....\n");
}

void display_stats(int argc, char* argv[]) {
    int visual = 0;
    int blockValue = 0;
    char visArr[50];
    for (int i = 0; i < 50; ++i) {
        visArr[i] = '.';
    }

    // if -v visual 
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0) {
            visual = 1;
            blockValue = MAX / 50;
        }
        else {
            printf("[error] Incorrect usage of STAT command. Try: %s -v", argv[0]);
        }
    }

    // print allocated memory stats
    printf("\n~ Allocated Memory ~\n");
    
    MemNode* curr = head;
    int totalAllocated = 0;
    int holeNum = 0;
    int largestHole = 0;
    
    while (curr != NULL) { // print all processes
        int size = curr->end - curr->start + 1;
        totalAllocated += size;
        printf("Process %s: Start = %d KB, End = %d KB, Size = %i KB\n", curr->process, curr->start, curr->end, size);

        // if visualization is enabled, keep track of it
        if (visual == 1) {
            // blocks for visualization
            int b_start = curr->start / blockValue;
            int b_end = curr->end / blockValue;

            for (int i = b_start; i <= b_end; ++i) {
                visArr[i] = '#'; 
            }
        }
        
        curr = curr->next;
    }

    // print free memory stats
    printf("\n~ Free Memory ~\n");
    // check if no head
    if (head == NULL) {
        printf("Hole 1: Start = %d KB, End = %d KB, Size = %d KB\n", 0, MAX - 1, MAX);
        holeNum = 1;
        largestHole = MAX;
    } else { // if there is a head
        // check before head
        if (head->start > 0) {
            int firstHoleSize = head->start;
            printf("Hole %d: Start = %d KB, End = %d KB, Size = %d KB\n", ++holeNum, 0, head->start - 1, firstHoleSize);
            if(firstHoleSize > largestHole){
                largestHole = firstHoleSize;
            }
        }
        
        // iterate through all mem nodes after head
        int last_end = head->end;
        curr = head->next;
        while (curr != NULL) {
            int holeSize = curr->start - last_end - 1;
            if (holeSize > 0) {
                printf("Hole %d: Start = %d KB, End = %d KB, Size = %d KB\n", ++holeNum, last_end + 1, curr->start - 1, holeSize);
                if (holeSize > largestHole) largestHole = holeSize;
            }

            last_end = curr->end;
            curr = curr->next;
        }

        // get last hole size if exists
        if (last_end < (MAX - 1)) {
            printf("Hole %d: Start = %d KB, End = %d KB, Size = %d KB\n", ++holeNum, last_end + 1, MAX - 1, (MAX - 1 - last_end));
        }
    }
    
    // print summary
    printf("\n~ Summary ~\n");
    
    // get the variables we need here
    int totalFree = MAX - totalAllocated;
    float externalFrag = (totalFree > 0) ? (1.0 - (float)largestHole / totalFree) * 100 : 0;
    int avgHoleSize = (holeNum > 0) ? (totalFree / holeNum) : 0;
    
    printf("Total allocated: %d KB\n", totalAllocated);
    printf("Total free: %d KB\n", totalFree);
    printf("Largest hole: %d KB\n", largestHole);
    printf("External fragmentation %.2f%%\n", externalFrag);
    printf("Average hole size: %d KB\n", avgHoleSize);

    if(visual == 1) {
        // visualization (optional)
        printf("\n~ Visualization / Memory Map Output ~\n");
        printf("[");
        for (int i = 0; i < 50; ++i) {
            printf("%c", visArr[i]);
        }
        printf("]\n");

        // print labels
        printf("^0 KB");
        for (int i = 0; i < 46; ++i) {
            printf(" ");
        }
        printf("^%d KB", MAX);
        printf("\n");
    }
}

int main (int argc, char* argv[]) {
    if(argc != 2) {
        printf("Incorrect usage. Try: %s <MAX_SIZE>\n", argv[0]);
        return -1; /// return that there was an error
    }

    // store the max value that they passed in
    MAX = atoi(argv[1]);

    char* args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    
    char input[MAX_LINE]; // input buffer for fgets
    char* tok; // temporary storage for tokens

    pid_t pid; // for child tracking
    int concurrent = 0; // concurrency counter (reset inside loop)
    
    while (should_run) {
        printf ("allocator› ");
        fflush (stdout);
        /* After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait unless command included &
        */
        
        // get input from stdin
        fgets(input, MAX_LINE, stdin);

        // trim input newline
        input[strcspn(input, "\n")] = 0;

        // separate out words from the input
        tok = strtok(input, " ");
        
        int i = 0;
        while(tok != NULL) { // iterate until no more args
            args[i] = tok;
            tok = strtok(NULL, " ");
            ++i;
        }
        args[i] = NULL; // ends the args array with null
        
        // if empty command
        if(i < 1) {
            continue;
        }

        // Handle exit command ("X") from user
        if(strcmp(args[0], "X") == 0) {
            exit(0);
        }

        // handle commands
        if (strcmp(args[0], "RQ") == 0) request_memory(i, args);
        else if (strcmp(args[0], "RL") == 0) release_memory(i, args);
        else if (strcmp(args[0], "C") == 0) compact();
        else if (strcmp(args[0], "STAT") == 0) display_stats(i, args);
        else {
            printf("[error] Unknown command: %s\n", args[0]);
        }
    }

    // always return something
    return 0;
}
