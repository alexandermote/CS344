#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/time.h>

// gcc -g -Wall -o mm2 mm2.c

#ifndef MIN_MATRIX_SIZE
# define MIN_MATRIX_SIZE 100
#endif // MIN_MATRIX_SIZE

#define COMMAND_ARGS "vho:s:"
#define MICROSECONDS_PER_SECOND 1000000.0

int matrix_size = MIN_MATRIX_SIZE;
int ver = 0;

float **lmatrix = NULL;
float **rmatrix = NULL;
float **mult_matrix = NULL;

void alloc_matrices(void) {
    int row = -1;
    if(ver) {
        fprintf(stderr, "%d: allocating matrices\n", __LINE__);
    }
    lmatrix = malloc(matrix_size * sizeof(float *));
    rmatrix = malloc(matrix_size * sizeof(float *));
    mult_matrix = malloc(matrix_size * sizeof(float *));
    for (row = 0; row < matrix_size; row++) {
        lmatrix[row] = malloc(matrix_size * sizeof(float));
        rmatrix[row] = malloc(matrix_size * sizeof(float));
        mult_matrix[row] = malloc(matrix_size * sizeof(float));
    }
    if (ver > 1) {
        fprintf(stderr, "%d: allocation complete\n", __LINE__);
    }
}

void free_matrices(void) {
    int row = -1;
    if(ver) {
        fprintf(stderr, "%d: deallocating matrices\n", __LINE__);
    }
    for (row = 0; row < matrix_size; row++) {
        free(lmatrix[row]);
        free(rmatrix[row]);
        free(mult_matrix[row]);
    }
    free(lmatrix);
    free(rmatrix);
    free(mult_matrix);
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
            lmatrix[row][col] = (row + col) * 2.0;
            rmatrix[row][col] = (row + col) * 3.0;
            mult_matrix[row][col] = 0.0;
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
            fprintf(ofile, "%.2f", mult_matrix[row][col]);
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
                       "%.9s: direct output (defaults to stdout)\n"
                       , argv[0]
                       , "-v"
                       , "-h"
                       , "-s [#]"
                       , "-o [file]");
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

    fprintf(stderr, "at\t\tit\t\tmm\t\tot\t\tdt\t\tmults/sec\tsize\n");
    fprintf(stderr, "%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.4lf\t%d\n"
            , alloc_time, init_time, mm_time, op_time, dealloc_time
            , mmults, matrix_size);
    }

    return(EXIT_SUCCESS);
}
