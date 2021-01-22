#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_struct.h"

// ./bin2csv -i SampleData_test.bin -o SampleData_test.csv
// ./bin2csv < SampleData_test.bin > SampleData_test.csv

#define LINE_MAX 1024

int main(int argc, char *argv[]) {
    int opt;
    int ver = FALSE;
    FILE *ofile = stdout;
    char *input;
    int inp = 0;
    char buf[LINE_MAX];
    file_struct_t conv;
    int bread;

    // used in verbose mode                                                          
    char *iname;
    char *oname;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch(opt){
        case 'h':
            printf("%s:\n\t"
                   "%9s: name of input file (defaults to stdin)\n\t"
                   "%9s: name of output file (defaults to stdout)\n\t"
                   "%9s: verbose processing\n\t"
                   "%9s: displays this help text\n"
                   , argv[0]
                   , "-i <file>"
                   , "-o <file>"
                   , "-v"
                   , "-h");
            exit(EXIT_SUCCESS);
        case 'i':
            input = optarg;
            inp = open(input, O_RDONLY, 0);
            if (inp == -1) {
                perror("Unable to open input file");
                exit(2);
            }
            if (ver) {
                iname = optarg;
                fprintf(stderr, "verbose: input file name: %s\n", iname);
            }
            break;
        case 'o':
            ofile = fopen(optarg, "w");
            if (ofile == NULL) {
                perror("Unable to open output file");
                exit(3);
            }
            if (ver) {
                oname = optarg;
                fprintf(stderr, "verbose: output file name: %s\n", oname);
            }
            break;
        case 'v':
            ver = TRUE;
            fprintf(stderr, "verbose: verbose enabled\n");
            break;
        }
    }

    if (ver) {
        if (iname == NULL) {
            strcpy(iname, "stdin");
        }
        if (oname == NULL) {
            strcpy(oname, "stdout");
        }
        fprintf(stderr, "verbose: using %s as input\n"
                "verbose: using %s as output\n", iname, oname);
    }

    fputs("id,first_name,middle_name,last_name,street,city,zip,country,email,phone\n", ofile); // place header file

    bread = read(inp, buf, NAME_LEN);

    while(bread != 0 && bread != -1) {
        // place strings into struct
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

        fprintf(ofile, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n"
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
        bread = read(inp, buf, NAME_LEN);
    }

    if (ofile != stdout) {
        fclose(ofile);
    }

    if (inp != 0) {
        close(inp);
    }

    return(EXIT_SUCCESS);
}
