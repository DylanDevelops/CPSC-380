/** 
 * Full Name: Dylan M. Ravel
 * Student ID: 2445987
 * Chapman Email: ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Syscall Interface
*/

#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    // declare variables
    int fin, fout;
    int nbytes;
    char buffer[1024];

    // make sure the correct number of arguments are passed in
    if(argc != 3) {
        write(2, "Usage: filecopy srcfile dstfile\n", 33);
        return -1;
    }

    // open the input file
    fin = open(argv[1], O_RDONLY);

    // check if fin failed to open
    if(fin == -1) {
        write(2, "Error opening read file\n", 25);
        return -1;
    }

    // open the output file
    fout = open(argv[2], O_CREAT | O_WRONLY, 0644);

    // check if fout failed to open
    if(fout == -1) {
        write(2, "Error opening write file\n", 26);
        close(fin);
        return -1;
    }

    // write to the output file
    while((nbytes = read(fin, buffer, 1024)) > 0) {
        write(fout, buffer, nbytes);
    }

    // close input and output file
    close(fin);
    close(fout);
}