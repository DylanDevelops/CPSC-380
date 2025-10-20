## AUTHOR INFO

Full Name: Dylan Ravel and Daniel Tsivkovski

Chapman Email: ravel@chapman.edu and tsivkovski@chapman.edu

Course Number and section: CPSC-380-02

Assignment or Exercise Number: Assignment 3 - Reader-Writer Synchronization

## ERRORS

We have not observed any errors from our testing.

## SOURCES

Below are all the sources that we used when coding this project.

- [Malloc Man Page](https://man7.org/linux/man-pages/man3/malloc.3.html)
- [Signal](https://man7.org/linux/man-pages/man3/signal.3p.html)
- [fprintf and snprintf](https://man7.org/linux/man-pages/man3/fprintf.3p.html)

## RUNNING INSTRUCTIONS

To compile, use `gcc -o rw_log rw_main.c -pthread`. This specifies the output file to include pthreads and names the output as `rw_log`.

To run the program, use `./rw_log -c <capacity> -r <num_readers> -w <num_writers> -b <writer_batch> -s <seconds> -R <reader_us> -W <writer_us> -d -h` where the flags are as follows:

- `-c <capacity>`: Capacity of the shared buffer.
- `-r <num_readers>`: Number of reader threads.
- `-w <num_writers>`: Number of writer threads.
- `-b <writer_batch>`: Number of writes each writer performs before sleeping.
- `-s <seconds>`: Total time in seconds for the program to run.
- `-R <reader_us>`: Microseconds each reader sleeps between read attempts.
- `-W <writer_us>`: Microseconds each writer sleeps between write attempts.
- `-d`: Dump the log to a CSV file named `log.csv`.
- `-h`: Displays help message.

## EXAMPLE

An example of using this program is:

`./rw_log -c 1024 -r 6 -w 4 -b 2 -s 12 -R 1000 -W 2000 -d`

This would set the values to:

- Buffer capacity of 1024
- 6 reader threads
- 4 writer threads
- Each writer performs 2 writes before sleeping
- Program runs for 12 seconds
- Each reader sleeps for 1000 microseconds between read attempts
- Each writer sleeps for 2000 microseconds between write attempts
- Dumps the log to `log.csv`
