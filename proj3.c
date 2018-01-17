// Amber Mickler
// CDA3101 Comp Org 2
// Project 3
// Cache Memory Simulator

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    int tag;
    int index;
    int isValid;
    int isDirty;
    int counter;
} cacheLine;

int main(int numargs, char **args) {
    int bsize, numsets, assoc, offset, index, tag;

    if (numargs != 7) {
        printf("Usage: proj3 <args> < <tracefile> \n\t -b: block size in bytes \n\t -s: number of sets \n\t -n: associativity of cache \n");
        return 0;
    }

    // read in flags & params
    int i = 0;
    for (i = 1; i < numargs; i+=2) {
        // size of block in bytes
        if (strstr(args[i], "-b")) {
            bsize = atoi(args[i+1]);
        }
        // number of sets
        else if (strstr(args[i], "-s")) {
            numsets = atoi(args[i+1]);
        }
        // associativity of cache
        else if (strstr(args[i], "-n")) {
            assoc = atoi(args[i+1]);
        }
        else {
            printf("Usage: proj3 <args> < <tracefile> \n\t -b: block size in bytes \n\t -s: number of sets \n\t -n: associativity of cache \n");
            return 0;
        }
    }
    offset = log(bsize)/log(2);
    index = log(numsets)/log(2);
    tag = 32 - offset - index;
    printf("Block size: %i \nNumber of sets: %i \nAssociativity: %i \nNumber of offset bits: %i\nNumber of index bits: %i \nNumber of tag bits: %i\n", bsize, numsets, assoc, offset, index, tag);

    int numref = 0, hit = 0, miss = 0, memref = 0;
    int numrefWA = 0, hitWA = 0, missWA = 0, memrefWA = 0;
    int flag = 0, flagWA = 0;
    int count;

    // create cache array
    int size = numsets*assoc;
    cacheLine cache[size];
    cacheLine cache2[size];

    for (i = 0; i < size; i++) {
        cache[i].tag = 0;
        cache[i].isValid = 0;
        cache[i].isDirty = 0;
        cache[i].counter = 0;
        cache2[i].tag = 0;
        cache2[i].isValid = 0;
        cache2[i].isDirty = 0;
        cache2[i].counter = 0;
        cache[i].index = 0;
        cache2[i].index = 0;
    }

    char line[16] = " ";
    unsigned int address;
    unsigned int chktag;
    unsigned int chkindex;
    while (fgets(line, 16, stdin)) {
        flag = 0;
        flagWA = 0;
        numref++;
        numrefWA++;
        // If Read
        if (line[0] == 'R') {
            strncpy(line, line+2, 16);
            address = atoi(line);
            chktag = address >> (32 - tag);
            chkindex = (address << tag) >> tag >> (32 - tag - index);

            //Read hit WTNWA
            //When a block is present in the cache (hit), a read simply grabs the data for the processor.
            for (i = 0; i < size; i++) {
                if (cache[i].isValid && (chktag == cache[i].tag) && (chkindex == cache[i].index)) {
                    hit++;
                    flag++;
                    cache[i].counter = -1;  // will be incremented to 0 below
                }
                cache[i].counter++;
            }
            //When a block is not present in the cache (miss), a read will cause the block to be pulled from main
                //memory into the cache, replacing the least recently used block if necessary.
            if (flag == 0) {
                count = 0;
                miss++;
                memref++;
                int oldest;
                for (i = 0; i < size; i++){
                    if ((cache[i].isValid !=0) && chkindex == cache[i].index) {
                        count++;
                        oldest = i;
                    }

                }
                if (count >= assoc) {
                    for (i = 0; i < size; i++) {
                        if (chkindex == cache[i].index) {
                            if (cache[i].counter > cache[oldest].counter)
                                oldest = i;
                        }
                    }
                }
                else {
                    oldest = 0;
                    for (i = 1; i < size; i++) {
                        if (cache[i].counter > cache[oldest].counter)
                            oldest = i;
                        }
                }

                cache[oldest].tag = chktag;
                cache[oldest].index = chkindex;
                cache[oldest].isValid = 1;
                cache[oldest].counter = 0;
            }

            // Read WBWA
            //When a block is present in the cache (hit), a read simply grabs the data for the processor.
            for (i = 0; i < size; i++) {
                if (cache2[i].isValid && (chktag == cache2[i].tag) && (chkindex == cache2[i].index)) {
                    hitWA++;
                    flagWA++;
                    cache2[i].counter = -1;
                }
                cache2[i].counter++;
            }
            //When a block is not present in the cache (miss), a read will cause the block to be pulled from main
                //memory into the cache, replacing the last recently used block if necessary. If the block being evicted
                //is dirty, the block’s contents must be written back to main memory.
            if (flagWA == 0) {
                count = 0;
                missWA++;
                memrefWA++;
                int oldestWA;
                for (i = 0; i < size; i++){
                    if ((cache2[i].isValid !=0) && chkindex == cache2[i].index) {
                        count++;
                        oldestWA = i;
                    }

                }
                if (count >= assoc) {
                    for (i = 0; i < size; i++) {
                        if (chkindex == cache2[i].index) {
                            if (cache2[i].counter > cache2[oldestWA].counter)
                                oldestWA = i;
                        }
                    }
                }
                else {
                    oldestWA = 0;
                    for (i = 1; i < size; i++) {
                        if (cache2[i].counter > cache2[oldestWA].counter)
                            oldestWA = i;
                        }
                }
                if (cache2[oldestWA].isDirty != 0)
                    memrefWA++;

                cache2[oldestWA].tag = chktag;
                cache2[oldestWA].index = chkindex;
                cache2[oldestWA].isValid = 1;
                cache2[oldestWA].isDirty = 0;
                cache2[oldestWA].counter = 0;
            }
        }

        // If Write
        if (line[0] == 'W') {
            strncpy(line, line+2, 16);
            address = atoi(line);
            chktag = address >> (32 - tag);
            chkindex = (address << tag) >> tag >> (32 - tag - index);

            // Write WTNWA
            //When a block is present in the cache (hit), a write will update both the cache and main memory (i.e.
                //we are writing through to main memory).
            for (i = 0; i < size; i++) {
                if (cache[i].isValid && (chktag == cache[i].tag) && (chkindex == cache[i].index)) {
                    hit++;
                    flag++;
                    memref++;
                    cache[i].counter = -1;
                }
                cache[i].counter++;
            }
            //When a block is not present in the cache (miss), a write will update the block in main memory but we
                //do not bring the block into the cache (this is why it is called “no write allocate”).
            if (flag == 0) {
                miss++;
                memref++;

            }
            // Write WBWA
            //When a block is present in the cache (hit), a write will update only the cache block and set the dirty
                //bit for the block. The dirty bit indicates that the cache entry is not in sync with main memory and will
                //need to be written back to main memory when the block is evicted from the cache.
            for (i = 0; i < size; i++) {
                if (cache2[i].isValid && (chktag == cache2[i].tag) && (chkindex == cache2[i].index)) {
                    hitWA++;
                    flagWA++;
                    cache2[i].counter = -1;
                    cache2[i].isDirty = 1;
                }
                cache2[i].counter++;
            }
            //When a block is not present in the cache (miss), a write will cause the block to be pulled from main
                //memory into the cache, replacing the least recently used block if necessary. When the block is pulled
                //into the cache, it should immediately be marked as “dirty”. If the block being evicted is dirty, the
                //block’s contents must be written back to main memory.
            if (flagWA == 0) {
                count = 0;
                missWA++;
                memrefWA++;
                int oldestWA;
                for (i = 0; i < size; i++){
                    if ((cache2[i].isValid !=0) && chkindex == cache2[i].index) {
                        count++;
                        oldestWA = i;
                    }
                }
                if (count >= assoc) {
                    for (i = 0; i < size; i++) {
                        if (chkindex == cache2[i].index) {
                            if (cache2[i].counter > cache2[oldestWA].counter)
                                oldestWA = i;
                        }
                    }
                }
                else {
                    oldestWA = 0;
                    for (i = 1; i < size; i++) {
                        if (cache2[i].counter > cache2[oldestWA].counter)
                            oldestWA = i;
                        }
                }
                if (cache2[oldestWA].isDirty != 0)
                    memrefWA++;

                cache2[oldestWA].tag = chktag;
                cache2[oldestWA].index = chkindex;
                cache2[oldestWA].isValid = 1;
                cache2[oldestWA].isDirty = 1;
                cache2[oldestWA].counter = 0;
            }
        }

    }

    printf("\n****************************************\nWrite-through with No Write Allocate\n****************************************\n");
    printf("Total number of references: %i \nHits: %i \nMisses: %i \nMemory References %i\n", numref, hit, miss, memref);

    printf("\n****************************************\nWrite-back with Write Allocate\n****************************************\n");
    printf("Total number of references: %i \nHits: %i \nMisses: %i \nMemory References %i\n", numrefWA, hitWA, missWA, memrefWA);

    return 0;
}
