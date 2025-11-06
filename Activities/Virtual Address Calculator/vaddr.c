#include <stdio.h>
#include <stdlib.h>

#define OFFSET_MASK 0x00000FFF

int main(int argc, char* argv[]) {
    uint32_t laddr;
    uint32_t page_no, offset;

    if(argc != 2) {
        printf("Usage: %s <laddr> \n", argv[0]);
        return -1;
    }

    laddr = (uint32_t)atoi(argv[1]);
    offset = laddr & OFFSET_MASK;

    page_no = laddr >> 12;

    printf("Offset: %d\n", offset);
    printf("Page No: %d\n", page_no);

    return 0;
}