#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_struct.h"

// ./bin2csv -i SampleData_test.bin -o SampleData_test.txt
// ./bin2csv -i SampleData_test.bin > SampleData_test.txt

#define LINE_MAX 1024

int main(int argc, char *argv[]) {
    int opt;
    FILE *ofile = stdout;
    char *input = NULL;
    char *output = NULL;
    int inp = 0;
    char buf[LINE_MAX];
    int bread;
    file_struct_t conv;
    int record;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch(opt){
        case 'h':
            printf("%s:\n\t"
                   "%9s: name of input file (REQUIRED)\n\t"
                   "%9s: name of output file (defaults to stdout)\n\t"
                   "%9s: displays this help text\n"
                   , argv[0]
                   , "-i <file>"
                   , "-o <file>"
                   , "-h");
            exit(EXIT_SUCCESS);
        case 'i':
            input = optarg;
            inp = open(input, O_RDONLY, 0);
            if (inp == -1) {
                fprintf(stderr, "Unable to open input file %s", input);
                exit(2);
            }
            break;
        case 'o':
            output = optarg;
            ofile = fopen(output, "w");
            if (ofile == NULL) {
                fprintf(stderr, "Unable to open output file %s", output);
                exit(2);
            }
            break;
        default:
            fprintf(stdout, "Invalid command line option\n");
            exit(7);
        }
    }

    if (input == NULL) {
        perror("No input file given");
        exit(3);
    }
    
    if (optind < argc) {
        int j;
        for (j = optind; j < argc; j++) {
            record = atoi(argv[j]);
            lseek(inp, record * sizeof(file_struct_t), SEEK_SET);
            bread = read(inp, buf, NAME_LEN);
            if (bread == 0 || bread == -1) {
                fprintf(stderr, "Record number %d not found\n", record);
            }
            else {
                strcpy(conv.fname, buf);
                read(inp, buf, NAME_LEN);
                strcpy(conv.mname, buf);
                read(inp, buf, NAME_LEN);
                strcpy(conv.lname, buf);
                read(inp, buf, STREEN_LEN);
                strcpy(conv.street, buf);
                read(inp, buf, CITY_LEN);
                strcpy(conv.city, buf);
                read(inp, buf, ZIP_LEN);
                strcpy(conv.zip, buf);
                read(inp, buf, COUNTRY_LEN);
                strcpy(conv.country_code, buf);
                read(inp, buf, EMAIL_LEN);
                strcpy(conv.email, buf);
                read(inp, buf, PHONE_LEN);
                strcpy(conv.phone, buf);
                read(inp, buf, ID_LEN);
                strcpy(conv.id, buf);
                
                fprintf(ofile, "\nid: %s\n "
                        "first_name: %s\n "
                        "middle_name: %s\n "
                        "last_name: %s\n "
                        "street: %s\n "
                        "city: %s\n "
                        "zip: %s\n "
                        "country: %s\n "
                        "email: %s\n "
                        "phone: %s\n"
                        , conv.id
                        , conv.fname
                        , conv.mname
                        , conv.lname
                        , conv.street
                        , conv.city
                        , conv.zip
                        , conv.country_code
                        , conv.email
                        , conv.phone);
                
                memset(&conv, 0, sizeof(file_struct_t));
                lseek(inp, 0, SEEK_SET);
            }
        }
    }
    else {
        while (1) {
            char *grab = malloc(sizeof(char *));
            fprintf(stdout, "Please enter the record number you wish to look up (q to quit): ");
            fgets(grab, LINE_MAX, stdin);
            if  (strcmp(grab, "q\n") == 0) {
                free(grab);
                return(EXIT_SUCCESS);
            }
            record = atoi(grab);
            lseek(inp, record * sizeof(file_struct_t), SEEK_SET);
            bread = read(inp, buf, NAME_LEN);
            if (bread == 0 || bread == -1) {
                fprintf(stderr, "Record number %d not found\n", record);
            }
            else {
                strcpy(conv.fname, buf);
                read(inp, buf, NAME_LEN);
                strcpy(conv.mname, buf);
                read(inp, buf, NAME_LEN);
                strcpy(conv.lname, buf);
                read(inp, buf, STREEN_LEN);
                strcpy(conv.street, buf);
                read(inp, buf, CITY_LEN);
                strcpy(conv.city, buf);
                read(inp, buf, ZIP_LEN);
                strcpy(conv.zip, buf);
                read(inp, buf, COUNTRY_LEN);
                strcpy(conv.country_code, buf);
                read(inp, buf, EMAIL_LEN);
                strcpy(conv.email, buf);
                read(inp, buf, PHONE_LEN);
                strcpy(conv.phone, buf);
                read(inp, buf, ID_LEN);
                strcpy(conv.id, buf);
                
                fprintf(ofile, "\nid: %s\n "
                        "first_name: %s\n "
                        "middle_name: %s\n "
                        "last_name: %s\n "
                        "street: %s\n "
                        "city: %s\n "
                        "zip: %s\n "
                        "country: %s\n "
                        "email: %s\n "
                        "phone: %s\n"
                        , conv.id
                        , conv.fname
                        , conv.mname
                        , conv.lname
                        , conv.street
                        , conv.city
                        , conv.zip
                        , conv.country_code
                        , conv.email
                        , conv.phone);
                
                memset(&conv, 0, sizeof(file_struct_t));
                lseek(inp, 0, SEEK_SET);
            }
            free(grab);
        }
    }
    
    if (ofile != stdout) {
        fclose(ofile);
    }
    
    if (inp != 0) {
        close(inp);
    }
    
    return(EXIT_SUCCESS);
}
