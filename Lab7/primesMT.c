#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdint.h>
#include <pthread.h>

#ifndef MAX_THREADS
# define MAX_THREADS 16
#endif //MAX_THREADS

#define COMMAND_ARGS "vhu:t:"

typedef struct BitBlock_s {
    uint32_t *bits;
    pthread_mutex_t mutex;
} BitBlock_t;
// mutexes must be initialized immediately upon creation
// let's define a macro to make that easier
#define INIT_BITBLOCK(_x) BitBlock_t _x = {.bits = 0x0, .mutex = PTHREAD_MUTEX_INITIALIZER}

int getnextnum(void);
void *thread_func(void *);

int ver = 0;
int num_threads = 1;
unsigned int max_num = 10240;
int *array;
int size;
BitBlock_t *blockarray;

int getnextnum(void) {
    static int next_num = 0;
    int val = 0;
    int arr_pos;
    int bit_pos;
    unsigned int flag;

    while(1) { // run until we find the next value
        // find next_num's position in the bit array
        arr_pos = next_num / 32;
        bit_pos = next_num % 32;
        // test that position for primality
        flag = 1;
        flag = flag << bit_pos;
        if (array[arr_pos] & flag) {
            // we got a prime!
            val = next_num;
            next_num++;
            return val;
        }
        next_num++;
    }
}

void *thread_func(void *arg) {
    int tid = (int) ((long) arg);
    int i = -1;
    int j = -1;
    if(ver) {
        fprintf(stderr, "%d: thread %2d starting\n"
                , __LINE__, tid);
    }

    for (i = getnextnum(); i*i < max_num; i = getnextnum()) {
        // remove all multiples >= square
        int mult;
        for (mult = i*i; mult < max_num; mult += i) {
            // just like getnextnum, except...
            int a = mult/32;
            int b = mult%32;
            unsigned int  f = 1;
            f = f << b;
            // ...we clear the bit
            f = ~f;
            for (j = 0; j < size; j++) {
                if (&(array[a]) == (int  *) blockarray[j].bits) {
                    pthread_mutex_lock(&(blockarray[j].mutex));
                    array[a] = array[a] & f;
                    pthread_mutex_unlock(&(blockarray[j].mutex));
                }
            }
        }
    }
    if (ver > 1) {
        fprintf(stderr, "%d: thread %2d complete\n"
                , __LINE__, tid);
    }
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    { // getopts
        int opt;
        while ((opt = getopt(argc, argv, COMMAND_ARGS)) != -1) {
            switch(opt) {
            case 'v':
                ver++;
                break;
            case 'h':
                printf("%s\n  "
                       "%.6s: verbose mode\n  "
                       "%.6s: display this help menu\n  "
                       "%.6s: change upper bound (defaults to 10240)\n  "
                       "%.6s: change thread count (defaults to 1, no more than 16)\n"
                       , argv[0]
                       , "-v"
                       , "-h"
                       , "-u [#]"
                       , "-t [#]");
                exit(EXIT_SUCCESS);
            case 'u':
                max_num = atoi(optarg);
                if(ver) {
                    fprintf(stderr, "%d: setting upper bound to %d\n"
                            , __LINE__
                            , max_num);
                }
                break;
            case 't':
                num_threads = atoi(optarg);
                if (ver) {
                    fprintf(stderr, "%d: thread count set to %d\n", __LINE__, num_threads);
                }
                if (num_threads < 1) {
                    num_threads = 1;
                    fprintf(stderr, "%d: thread count reset to %d\n", __LINE__, num_threads);
                }
                if (num_threads > MAX_THREADS) {
                    num_threads = MAX_THREADS;
                    fprintf(stderr, "%d: thread count reset to %d\n", __LINE__, num_threads);
                }
                break;
            default:
                break;
            }
        }
    }

    { // array and mutex initialization
        int i;
        unsigned int bitstart;
        size = max_num / 32;
        if (max_num % 32 != 0) size++;
        array = malloc(size * sizeof(int));
        for (i = 0; i < size; i++) { // initialize to all 1's
            array[i] = -1;
        }
        // we know 0 and 1 are not primes
        bitstart = 3;
        bitstart = ~bitstart;
        array[0] = array[0] & bitstart;

        blockarray = malloc(sizeof(BitBlock_t));
        for (i = 0; i < size; i++) {
            INIT_BITBLOCK(block);
            blockarray = realloc(blockarray, (i+1) * sizeof(BitBlock_t));
            block.bits = (uint32_t *) &(array[i]);
            blockarray[i] = block;
        }
    }

    { // thread management
        pthread_t *wthreads = NULL;
        long tid = 0;

        wthreads = calloc(num_threads, sizeof(pthread_t));
        for (tid = 0; tid < num_threads; tid++) {
            pthread_create(&wthreads[tid], NULL, thread_func, (void *) tid);
        }
        for (tid = 0; tid < num_threads; tid++) {
            pthread_join(wthreads[tid], NULL);
        }
        free(wthreads);
    }

    { // print primes
        int i;
        for (i = 0; i < max_num; i++) {
            // check for primes using the now-familiar process
            int ap = i/32;
            int bp = i%32;
            unsigned int fp = 1;
            fp = fp << bp;
            if (array[ap] & fp) {
                printf("%d\n", i);
            }
        }
    }

    { // free dynamic memory and destroy mutexes
        int i;
        free(array);
        for (i = 0; i < size; i++) {
            pthread_mutex_destroy(&(blockarray[i].mutex));
        }
        free(blockarray);
    }

    return(EXIT_SUCCESS);
}
