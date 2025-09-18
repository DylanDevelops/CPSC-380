/** 
 * Full Name: Dylan Ravel and Daniel Tsivkovski
 * Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 1 - Simple Shell Interface
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */

int main (void) {
    char* args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    
    char input[MAX_LINE]; // input buffer for fgets
    char* tok; // temporary storage for tokens
    int concurrent = 0;

    pid_t pid; // for child tracking
    
    while (should_run) {
        printf ("osh› ");
        fflush (stdout);
        /* After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait unless command included &
        */

        // reset concurrency value
        concurrent = 0;
        
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
        
        // once done check for '&' as last token
        if (args[i-1][0] == '&') {
            args[i-1] = NULL; // remove & from command
            concurrent = 1; // enable concurrency
            --i;
        }

        // Handle "exit" command from user
        if(strcmp(args[0], "exit") == 0) {
            should_run = 0;
        }

        // fork to create a child
        pid = fork();
        
        // handle depending on process
        if (pid < 0) {
            // if error occurs while forking (erectile dysfunction)
            printf("[ERROR] Command failed to run. Please try again.");
        } else if (pid == 0) { // successful fork (and it's a child process)
            // execute command as a child
            execvp(args[0], args);

            // if execvp fails, exit with an error
            exit(1);
        } else { // handle parent process
            // if not concurrent then parent process must wait
            if (!concurrent) {
                wait(NULL);
            }
        }
    }

    // always return something
    return 0;
}