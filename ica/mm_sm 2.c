#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

// gcc -g -Wall -lrt -o mm_sm mm_sm.c

#ifndef MIN_MATRIX_SIZE
# define MIN_MATRIX_SIZE 100
#endif // MIN_MATRIX_SIZE

#ifndef MAX_PROCS
# define MAX_PROCS 16
#endif //MAX_PROCS

#define COMMAND_ARGS "vho:s:p:"
#define MICROSECONDS_PER_SECOND 1000000.0
#define ELEMENT(_mat,_x,_y,_cols) _mat[(_x * _cols) + _y]
#define LINE_MAX 1024

int matrix_size = MIN_MATRIX_SIZE;
int ver = 0;
char shm_name[LINE_MAX];
int shmfd = 1;

float *lmatrix = NULL;
float *rmatrix = NULL;
float *mult_matrix = NULL;

void alloc_matrices(void) {
    // int row = -1;
    if(ver) {
        fprintf(stderr, "%d: allocating matrices\n", __LINE__);
    }
    lmatrix = malloc(matrix_size * matrix_size * sizeof(float));
    rmatrix = malloc(matrix_size * matrix_size * sizeof(float));
    // shared memory matrix
    sprintf(shm_name, "%s.%s", "/SharedMemMatrixMult", getenv("LOGNAME"));
    if (shm_unlink(shm_name) != 0) {
        // fprintf(stderr, "could not unlink shm file >%s<\n", shm_name);
        // hopefully we won't ever need this
    }
    shmfd = shm_open(shm_name
                     , (O_CREAT | O_RDWR | O_EXCL)
                     , (S_IRUSR | S_IWUSR));
    if (shmfd < 0) {
        fprintf(stderr, "Failed to open/create shared memory segment >%s<\n"
                , shm_name);
        exit(EXIT_FAILURE);
    }
    ftruncate(shmfd, matrix_size * matrix_size * sizeof(float));
    mult_matrix = (float *) mmap(NULL
                            , matrix_size * matrix_size * sizeof(float)
                            , PROT_READ | PROT_WRITE
                            , MAP_SHARED
                            , shmfd
                            , 0);
    /* non-shared memory matrix
    mult_matrix = malloc(matrix_size * sizeof(float *));
    for (row = 0; row < matrix_size; row++) {
        lmatrix[row] = malloc(matrix_size * sizeof(float));
        rmatrix[row] = malloc(matrix_size * sizeof(float));
        mult_matrix[row] = malloc(matrix_size * sizeof(float));
    }
    */
    if (ver > 1) {
        fprintf(stderr, "%d: allocation complete\n", __LINE__);
    }
}

void free_matrices(void) {
    if(ver) {
        fprintf(stderr, "%d: deallocating matrices\n", __LINE__);
    }
    free(lmatrix);
    free(rmatrix);

    // free(mult_matrix);
    munmap(mult_matrix, matrix_size * matrix_size * sizeof(float));
    close(shmfd);
    shm_unlink(shm_name);

    lmatrix = rmatrix = mult_matrix = NULL;
    if (ver > 1) {
        fprintf(stderr, "%d: deallocation complete\n", __LINE__);
    }
}

void init_matrices(void) {
    int row = -1;
    int col = -1;
   
    if(ver) {
        fprintf(stderr, "%d: initializing matrices\n", __LINE__);
    }
    for (row = 0; row < matrix_size; row++) {
        for (col = 0; col < matrix_size; col++) {
            // lmatrix[row][col] = (row + col) * 2.0;
            ELEMENT(lmatrix,row,col,matrix_size) = (row + col) * 2.0;
            // rmatrix[row][col] = (row + col) * 3.0;
            ELEMENT(rmatrix,row,col,matrix_size) = (row + col) * 3.0;
            // mult_matrix[row][col] = 0.0;
            ELEMENT(mult_matrix,row,col,matrix_size) = 0.0;
        }
    }
    if (ver > 1) {
        fprintf(stderr, "%d: initialization complete\n", __LINE__);
    }
}

void op_result(char *name) {
    int row = -1;
    int col = -1;
    FILE *ofile = stdout;

    if(ver) {
        fprintf(stderr, "%d: output result to file %s\n"
                , __LINE__
                , name ? name : "stdout");
    }
    if (name != NULL) {
        ofile = fopen(name, "w");
        if (ofile == NULL) {
            perror("could not open output file");
            fprintf(stderr, "Failed to open output file %s\n", name);
            exit(1);
        }
    }

    for (row = 0; row < matrix_size; row++) {
        for (col = 0; col < matrix_size; col++) {
            fprintf(ofile, "%.2f", ELEMENT(mult_matrix,row,col,matrix_size));
        }
        fprintf(ofile, "\n");
    }
    if (name != NULL) {
        fclose(ofile);
    }
    if (ver > 1) {
        fprintf(stderr, "%d: output complete\n", __LINE__);
    }
}

double elapse_time(struct timeval *t0, struct timeval *t1) {
    double et = (((double) (t1->tv_usec - t0->tv_usec)) / MICROSECONDS_PER_SECOND)
        + ((double) (t1->tv_sec - t0->tv_sec));
    return et;
}
int main(int argc, char *argv[]) {
    char *filename = NULL;
    int num_procs = 1;
    int proc = -1;
    pid_t cpid = -1;
    struct timeval alloc_time0;
    struct timeval alloc_time1;
    struct timeval init_time0;
    struct timeval init_time1;
    struct timeval mm_time0;
    struct timeval mm_time1;
    struct timeval op_time0;
    struct timeval op_time1;
    struct timeval dealloc_time0;
    struct timeval dealloc_time1;

    {
        int opt;
        while ((opt = getopt(argc, argv, COMMAND_ARGS)) != -1) {
            switch(opt) {
            case 'v':
                ver++;
                break;
            case 'h':
                printf("%s\n\t"
                       "%.9s: verbose mode\n\t"
                       "%.9s: display this help menu\n\t"
                       "%.9s: change size of matrices (must be at least 100)\n\t"
                       "%.9s: direct output (defaults to stdout)\n\t"
                       "%.9s: change thread count (defaults to 1, must be less than 17)\n"
                       , argv[0]
                       , "-v"
                       , "-h"
                       , "-s [#]"
                       , "-o [file]"
                       , "-t [#]");
                exit(EXIT_SUCCESS);
            case 's':
                matrix_size = atoi(optarg);
                if(ver) {
                    fprintf(stderr, "%d: setting matrix to size %d\n"
                            , __LINE__
                            , matrix_size);
                }
                if (matrix_size < MIN_MATRIX_SIZE) {
                    matrix_size = MIN_MATRIX_SIZE;
                    if(ver) {
                        fprintf(stderr, "%d: resetting  matrix to size %d\n"
                                , __LINE__
                                , matrix_size);
                    }
                }
                break;
            case 'o':
                filename = optarg;
                if(ver) {
                    fprintf(stderr, "%d: output file set to %s\n"
                            , __LINE__
                            , filename);
                }
                break;
            case 'p':
                num_procs = atoi(optarg);
                if (ver) {
                    fprintf(stderr, "%d: process count set to %d\n", __LINE__, num_procs);
                }
                if (num_procs < 1) {
                    num_procs = 1;
                    fprintf(stderr, "%d: process count reset to %d\n", __LINE__, num_procs);
                }
                if (num_procs > MAX_PROCS) {
                    num_procs = MAX_PROCS;
                    fprintf(stderr, "%d: process count reset to %d\n", __LINE__, num_procs);
                }
                break;
            default:
                break;
            }
        }
    }
    gettimeofday(&alloc_time0, NULL);
    alloc_matrices();
    gettimeofday(&alloc_time1, NULL);

    gettimeofday(&init_time0, NULL);
    init_matrices();
    gettimeofday(&init_time1, NULL);

    gettimeofday(&mm_time0, NULL);
/*
  {
  int i = -1;
  int j = -1;
  int k = -1;
  
  for (i = 0; i < matrix_size; i++) {
  for (j = 0; j < matrix_size; j++) {
  for (k = 0; k < matrix_size; k++) {
  mult_matrix[i][j] += lmatrix[i][k] * rmatrix[k][j];
  }
  }
  }
  }
*/

    for (proc = 0; proc < num_procs; proc++) {
        cpid = fork();
        if (cpid == 0) {
            int i = -1;
            int j = -1;
            int k = -1;

            if (ver > 2) {
                fprintf(stderr, "\t%d: child process %d started\n", __LINE__, cpid);
            }
            for (i = proc; i < matrix_size; i += num_procs) {
                for (j = 0; j < matrix_size; j++) {
                    for (k = 0; k < matrix_size; k++) {
                        ELEMENT(mult_matrix,i,j,matrix_size) +=
                            (ELEMENT(lmatrix,i,k,matrix_size)
                             * ELEMENT(rmatrix,k,j,matrix_size));
                    }
                }
            }
            if (ver > 2) {
                fprintf(stderr, "\t%d: child process %d done\n", __LINE__, cpid);
            }
            fflush(stderr);
            _exit(0);
        }
    }
    while ((cpid = wait(NULL)) > 0) {
        if(ver) {
            fprintf(stderr, "\t%d: child process %d reaped\n", __LINE__, cpid);
        }
    }

    gettimeofday(&mm_time1, NULL);

    gettimeofday(&op_time0, NULL);
    op_result(filename);
    gettimeofday(&op_time1, NULL);

    gettimeofday(&dealloc_time0, NULL);
    free_matrices();
    gettimeofday(&dealloc_time1, NULL);

    {
        double alloc_time = elapse_time(&alloc_time0, &alloc_time1);
        double init_time = elapse_time(&alloc_time0, &alloc_time1);
        double mm_time = elapse_time(&mm_time0, &mm_time1);
        double op_time = elapse_time(&op_time0, &op_time1);
        double dealloc_time = elapse_time(&dealloc_time0, &dealloc_time1);
        double mmults = (matrix_size * matrix_size) / mm_time;

    fprintf(stderr, "at\t\tit\t\tmm\t\tot\t\tdt\t\tmults/sec\tsize\tprocs\n");
    fprintf(stderr, "%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.4lf\t%d\t%d\n"
            , alloc_time, init_time, mm_time, op_time, dealloc_time
            , mmults, matrix_size, num_procs);
    }

    return(EXIT_SUCCESS);
}
