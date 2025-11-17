## AUTHOR INFO

Full Name: Dylan Ravel and Daniel Tsivkovski

Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu

Course Number and section: CPSC-380-02

Assignment or Exercise Number: Assignment 5 - Continuous Memory Allocation

## ERRORS

We have not observed any errors from our testing.

## SOURCES

Below are all the sources that we used when coding this project.

- [File Handling](https://www.w3schools.com/c/c_files_read.php)
- [fopen](https://man7.org/linux/man-pages/man3/fopen.3.html)
- [strdup](https://man7.org/linux/man-pages/man3/strdup.3.html)
- 

## RUNNING INSTRUCTIONS

To compile, use `gcc -o schedsim schedsim.c -pthread`. This specifies the output file to include pthreads and names the output as `schedsim`.

To run the program, use `./schedsim [--fcfs|--sjf|--priority|--rr] --input <file> [--quantum <num>]` where the flags are as follows:

- `--fcfs` or `-f`: Use first come first serve scheduling.
- `--sjf` or `-s`: Use shortest job first scheduling.
- `--priority` or `-p`: Use priority scheduling.
- `--rr` or `-r`: Use round robin scheduling.
- `--input <file>` or `-i <file>`: Input CSV file containing process workload (required).
- `--quantum <num>` or `-q <num>`: Time quantum for round robin scheduling (default is 2).

## EXAMPLE

An example of using this program is:

`./schedsim --rr --quantum 4 --input workload.csv`

This would set the values to:

- Round robin scheduling algorithm
- Time quantum of 4
- Input file `workload.csv`
