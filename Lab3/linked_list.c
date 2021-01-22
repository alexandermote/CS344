#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"

// gcc -g -Wall -o linked_list linked_list.c
 
// ./linked_list ingredients1.c ingredients2.c

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    char buf[LINE_MAX] = {0};
    char *filename = NULL;

    node_t *head = malloc(sizeof(node_t));
    node_t *tail = malloc(sizeof(node_t));

    if (optind < argc) { // Loop 1: Create the list
        int j;
        for (j = optind; j < argc; j++) {
            filename = argv[j];
            ifile = fopen(filename, "r");
            if (ifile == NULL) {
                perror("Failed to open file.");
                fprintf(stderr, "  could not open file %s\n", filename);
            }
            else {
                while (fgets(buf, LINE_MAX, ifile) != NULL) {
                    if (head->line == NULL) {
                        head->line = strdup(buf);
                        head->next = tail;
                    }
                    else {
                        node_t *current_node = head;
                        node_t *prev_node;
                        node_t *add_node = malloc(sizeof(node_t));
                        add_node->line = strdup(buf);

                        while (current_node != tail) {
                            prev_node = current_node;
                            current_node = prev_node->next;
                        }
                        prev_node->next = add_node;
                        add_node->next = current_node;
                    }
                }
                fclose(ifile);
            }
        }
    }
    else {
        while(fgets(buf, LINE_MAX, ifile) != NULL) {
            if (head->line == NULL) {
                head->line = buf;
                head->next = tail;
            }
            else {
                node_t *current_node = head;
                node_t *prev_node;
                node_t *add_node = malloc(sizeof(node_t));
                add_node->line = strdup(buf);
                
                while (current_node != tail) {
                    prev_node = current_node;
                    current_node = prev_node->next;
                }
                prev_node->next = add_node;
                add_node->next = current_node;
            }
        }
        if (filename != NULL) {
            fclose(ifile);
        }
    }

    node_t *current_node = head; // Loop 2: Print the list
    node_t *prev_node;
    int listcount = 1;
    while (current_node->line != NULL) {
        char *val = current_node->line;
        printf("%03d: %s", listcount, val);
        prev_node = current_node;
        current_node = prev_node->next;
        listcount++;
    }
    
    node_t *free_node; // Loop 3: Free the list
    while(head != NULL) {
        free_node = head;
        head = head->next;
        free(free_node->line);
        free(free_node);
    }
    return(EXIT_SUCCESS);
}
