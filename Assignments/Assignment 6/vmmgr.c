/** 
 * Full Name: Daniel Tsivkovski and Dylan Ravel
 * Chapman Email: tsivkovski@chapman.edu, ravel@chapman.edu
 * Course Number and Section: CPSC-380-02
 * Assignment or Exercise Number: Assignment 6 - Virtual Memory Manager
*/

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256
#define PAGE_TABLE_SIZE 256
#define OFFSET_BITS 8
#define OFFSET_MASK 0xFF
#define TLB_SIZE 16
#define PHYSICAL_FRAMES 128

typedef struct {
    int frame_num;
    int valid;
} PageTableEntry;

typedef struct {
    PageTableEntry entries[PAGE_TABLE_SIZE];
} PageTable;

typedef struct {
    int page_num;
    int frame_num;
    int valid;
    int last_used;
} TLBEntry;

// global variables
unsigned char physical_memory[PHYSICAL_FRAMES * PAGE_SIZE];
int free_frame = 0;
PageTable page_table;
TLBEntry tlb[TLB_SIZE];
FILE* backing_store;
// page replacement queue
int pr_queue[PHYSICAL_FRAMES];
int pr_head = 0;

// function declarations
void clear_tlb_entry(int page_num);
void handle_page_fault(int page_num);
void init_page_table(PageTable *pt);
void init_tlb();
int tlb_search(int page_num, int timestamp);
void tlb_update(int page_num, int frame_num, int timestamp);

int main(int argc, char *argv[]) {
    // check command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // open input file
    FILE* addr_file = fopen(argv[1], "r");
    if (addr_file == NULL) {
        fprintf(stderr, "Error opening file. %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // open backing store
    backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL) {
        fprintf(stderr, "Error opening backing store.\n");
        fclose(addr_file);
        return EXIT_FAILURE;
    }

    // init page table and TLB
    init_page_table(&page_table);
    init_tlb();

    // set up variables for address translation and stats
    int logical_addr;
    int total_addr = 0;
    int page_faults = 0;
    int tlb_hits = 0;

    // process logical addresses
    while(fscanf(addr_file, "%d", &logical_addr) != EOF) {
        
        total_addr++; 

        // get page number
        int page_num = (logical_addr >> OFFSET_BITS) & 0xFF;
        // get offset
        int offset = logical_addr & OFFSET_MASK;

        // try TLB first
        int frame_num = tlb_search(page_num, total_addr);
        if (frame_num == -1) { // TLB miss
            // check page table 
            if (page_table.entries[page_num].valid == 0) {
                // page fault
                handle_page_fault(page_num);
                page_faults++;
            }
            // get frame number from page table
            frame_num = page_table.entries[page_num].frame_num;
            // update TLB
            tlb_update(page_num, frame_num, total_addr);
        } else { // TLB hit
            tlb_hits++;
        }

        // calculate physical address and retrieve value
        int physical_addr = (frame_num << OFFSET_BITS) | offset;
        unsigned char value = physical_memory[physical_addr];

        // print info for this address
        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_addr, physical_addr, value);
    }

    // check if there is an empty input file
    if(total_addr == 0) {
        printf("There are no addresses to process.\n");
        fclose(addr_file);
        fclose(backing_store);

        return EXIT_SUCCESS;
    }

    // print stats
    double page_fault_rate = (page_faults / (double)total_addr) * 100.0;
    double tlb_hit_rate = (tlb_hits / (double)total_addr) * 100.0;
    printf("Page Fault Rate: %.3f%%\n", page_fault_rate);
    printf("TLB Hit Rate: %.3f%%\n", tlb_hit_rate);

    // close files
    fclose(addr_file);
    fclose(backing_store);
    return EXIT_SUCCESS;
}

// init page table
void init_page_table(PageTable *pt) {
    // iterate through entire table
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        // set as not valid
        pt->entries[i].valid = 0;
        // frame number -1 to show no frame given
        pt->entries[i].frame_num = -1;
    }
}

void handle_page_fault(int page_num) {

    int frame_num;

    if (free_frame < PHYSICAL_FRAMES) {
        // use next free frame
        frame_num = free_frame++;
        // update queue with frame num
        pr_queue[frame_num] = page_num;
    } else {
        // physical memory full, evict
        
        // get frame to replace and find evicted page
        frame_num = pr_head;
        int evicted_page = pr_queue[pr_head];

        // clear evicted page table entry
        page_table.entries[evicted_page].valid = 0;

        // clear page from TLB
        clear_tlb_entry(evicted_page);

        // update queue with new page
        pr_queue[pr_head] = page_num;
        pr_head = (pr_head + 1) % PHYSICAL_FRAMES;
    }

    // calculate offset and seek
    int offset = page_num * PAGE_SIZE;
    fseek(backing_store, offset, SEEK_SET);
    
    // read page into physical memory
    fread(&physical_memory[frame_num * PAGE_SIZE], PAGE_SIZE, 1, backing_store);

    // update page table
    page_table.entries[page_num].frame_num = frame_num;
    page_table.entries[page_num].valid = 1;
}

void clear_tlb_entry(int page_num) {
    // iterate through all TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // if entry matches page num
        if (tlb[i].valid && tlb[i].page_num == page_num) {
            tlb[i].valid = 0; // invalidate entry
            return;
        }
    }
}

void init_tlb() {
    for (int i = 0; i < TLB_SIZE; i++) {
        // initialize all entries with invalid values
        tlb[i].page_num = -1;
        tlb[i].frame_num = -1;
        tlb[i].valid = 0;
        tlb[i].last_used = -1;
    }
}

int tlb_search(int page_num, int timestamp) {
    // iterate thorugh TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // check for valid entry and matching page num
        if (tlb[i].valid && tlb[i].page_num == page_num) {
            // update last used timestamp
            tlb[i].last_used = timestamp;
            return tlb[i].frame_num; // return frame number
        }
    }
    return -1; // if not found
}

void tlb_update(int page_num, int frame_num, int timestamp) {
    // find an empty slot or the least recently used entry
    int lru_pos = 0;
    int lru_time = tlb[0].last_used;

    // iterate through TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // check for empty/invalid slot
        if (!tlb[i].valid) {
            // update this slot
            tlb[i].page_num = page_num;
            tlb[i].frame_num = frame_num;
            tlb[i].valid = 1;
            tlb[i].last_used = timestamp;
            return;
        }
        // update least recently used info
        if (tlb[i].last_used < lru_time) {
            lru_time = tlb[i].last_used;
            lru_pos = i;
        }
    }

    // replace least recently used entry
    tlb[lru_pos].page_num = page_num;
    tlb[lru_pos].frame_num = frame_num;
    tlb[lru_pos].valid = 1;
    tlb[lru_pos].last_used = timestamp;
}