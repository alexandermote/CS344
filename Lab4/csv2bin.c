#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_struct.h"

// ./csv2bin -i SampleData.csv -o SampleData_test.bin
// ./csv2bin < SampleData.csv > SampleData_test.bin

#define LINE_MAX 1024

int main(int argc, char *argv[]) {
    int opt;
    int ver = FALSE;
    FILE *ifile = stdin;
    char *output;
    int out = 1;
    char buf[LINE_MAX] = {0};
    file_struct_t conv;

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
            ifile = fopen(optarg, "r");
            if (ifile == NULL) {
                perror("Unable to open input file");
                exit(2);
            }
            if (ver) {
                iname = optarg;
                fprintf(stderr, "verbose: input file name: %s\n", iname);
            }
            break;
        case 'o':
            output = optarg;
            out = creat(output, S_IRWXU|S_IRWXG);
            if (out == -1) {
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

    fgets(buf, LINE_MAX, ifile); // throw away header line
    while (fgets(buf, LINE_MAX, ifile) != NULL) { // iterate through each line
        // collect all the tokens to win a prize!
        char *id = strtok(buf, ",");
        char *fname = strtok(NULL, ",");
        char *mname = strtok(NULL, ",");
        char *lname = strtok(NULL, ",");
        char *street = strtok(NULL, ",");
        char *city = strtok(NULL, ",");
        char *zip = strtok(NULL, ",");
        char *country_code = strtok(NULL, ",");
        char *email = strtok(NULL, ",");
        char *phone = strtok(NULL, "\n");

        // truncate tokens if necessary
        if (strlen(id) > ID_LEN) id[ID_LEN] = '\0';
        if (strlen(fname) > NAME_LEN) fname[NAME_LEN] = '\0';
        if (strlen(mname) > NAME_LEN) mname[NAME_LEN] = '\0';
        if (strlen(lname) > NAME_LEN) lname[NAME_LEN] = '\0';
        if (strlen(street) > STREEN_LEN) street[STREEN_LEN] = '\0';
        if (strlen(city) > CITY_LEN) city[CITY_LEN] = '\0';
        if (strlen(zip) > ZIP_LEN) zip[ZIP_LEN] = '\0';
        if (strlen(country_code) > COUNTRY_LEN) country_code[COUNTRY_LEN] = '\0';
        if (strlen(email) > EMAIL_LEN) email[EMAIL_LEN] = '\0';
        if (strlen(phone) > PHONE_LEN) phone[PHONE_LEN] = '\0';

        // finally add tokens to struct
        strcpy(conv.id, id);
        strcpy(conv.fname, fname);
        strcpy(conv.mname, mname);
        strcpy(conv.lname, lname);
        strcpy(conv.street, street);
        strcpy(conv.city, city);
        strcpy(conv.zip, zip);
        strcpy(conv.country_code, country_code);
        strcpy(conv.email, email);
        strcpy(conv.phone, phone);

        // write contents of struct to bin and reset struct
        write(out, &conv, sizeof(file_struct_t));
        memset(&conv, 0, sizeof(file_struct_t));
    }

    if (ifile != stdin) {
        fclose(ifile);
    }
    if (out != 1) {
        close(out);
    }
    return(EXIT_SUCCESS);
}
