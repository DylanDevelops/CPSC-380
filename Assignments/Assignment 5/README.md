## AUTHOR INFO

Full Name: Dylan Ravel and Daniel Tsivkovski

Chapman Email: <ravel@chapman.edu> and <tsivkovski@chapman.edu>

Course Number and section: CPSC-380-02

Assignment or Exercise Number: Assignment 5 - Continuous Memory Allocation

## ERRORS

We have not observed any errors from our testing. Let's hope it stays that way! 🙏

## SOURCES

Below are all the sources that we used when coding this project.

- [File Handling](https://www.w3schools.com/c/c_files_read.php)
- [fopen](https://man7.org/linux/man-pages/man3/fopen.3.html)
- [strdup](https://man7.org/linux/man-pages/man3/strdup.3.html)
- [strcmp](https://man7.org/linux/man-pages/man3/strcmp.3.html)
- [Linked Lists in C](https://www.geeksforgeeks.org/c/c-program-to-implement-singly-linked-list/)

## RUNNING INSTRUCTIONS

**To compile:**
`gcc allocator.c -o allocator`

**To run the program, use:**
`./allocator <MAX_SIZE>`

**To use:**

- RQ <process> <size> <F | B | W>
  - Request allocation for a process of a set size in KB using:
    - F = First fit
    - B = Best fit
    - W = Worst fit
  - Example: `RQ P1 40000 F`

- RL <process>
  - Releases a process from memory
  - Example: `RL P1`

- C
  - Compacts memory moving all allocated blocks to eliminate left over holes
  - Example: `C`

- STAT [-v]
  - Prints all information on allocated memory, holes in memory, and fragmentation and usage statistics.
  - Use `-v` for a visualization
  - Example: `STAT -v`

- SIM <filename>
  - Executes commands from a trace file (one command per line)
  - Trace file commands follow same format as shown above
  - Example: `SIM trace.txt`

- X
  - Ends allocator program.
  - Example: `X`

Example Commands:

- `RQ P1 40000 F` - allocates 40000 KB to process P1 using the First Fit strategy, if possible
- `RL P2` - releases process P2 from memory
- `STAT -v` - prints statistics and visualization of memory usage
- `SIM trace.txt` - simulates all commands written out line by line in the trace.txt file
