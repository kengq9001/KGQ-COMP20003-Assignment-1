// wt_ops.c
// Reads, prints, frees watchtower information

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wt_ops.h"

#define MAX_LINE_LEN 512
#define WT_START_SIZE 10

// Reads watchtower information from the csv file and returns an array
// of wt_info_t pointers.
// Also updates the number of watchtowers as the file is read.
// realloc usage idea from https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/realloc.c
wt_info_t **ReadWtInfo(FILE *file, int *n) {
    wt_info_t **watchtowers;
    int currentSize = WT_START_SIZE;
    if ( (watchtowers = (wt_info_t**)malloc(sizeof(wt_info_t*)*currentSize)) == NULL ) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineBufferLength = MAX_LINE_LEN;

    // first row which is just headings
    getline(&line, &lineBufferLength, file);

    // remaining rows
    while (getline(&line, &lineBufferLength, file) > 0) {
        // reallocate space if not enough
        if (*n == currentSize) {
            currentSize*=2;
            if ( (watchtowers = (wt_info_t**)
            realloc(watchtowers, sizeof(wt_info_t*)*currentSize)) == NULL ) {
                printf("realloc() error\n");
                exit(EXIT_FAILURE);
            }
        }
        watchtowers[*n] = SplitLine(line);
        *n+=1;
    }
    
    free(line);
    return watchtowers;
}

// Split each line into 6 components and store each in the struct pointer
// Assuming the headings and their order is always the same
// strtok usage idea from https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
wt_info_t *SplitLine(char *line) {
    wt_info_t *info;
    if ( (info = (wt_info_t*)malloc(sizeof(wt_info_t))) == NULL ) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    char *token;

    token = strtok(line, ",");
    info->ID = OneString(token);

    token = strtok(NULL, ",");
    info->postcode = OneString(token);

    token = strtok(NULL, ",");
    info->population = atoi(token);

    token = strtok(NULL, ",");
    info->contact_name = OneString(token);

    token = strtok(NULL, ",");
    info->x = atof(token);

    token = strtok(NULL, "\n");
    info->y = atof(token);
    
    return info;
}

// Further string processing for string entries
char *OneString(char *line) {
    int len;
    len = strlen(line);
    char *temp;
    if ( (temp = (char*)malloc(sizeof(char)*(len+1))) == NULL ) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    strcpy(temp, line);
    temp[len] = '\0';
    return temp;
}

// Outputs one watchtower information into a file
void PrintWtInfo(FILE *file, wt_info_t *entry) {
    fprintf(file, "Watchtower ID: %s, Postcode: %s, Population Served: %d, "
           "Watchtower Point of Contact Name: %s, x: %f, y: %f\n",
    entry->ID,entry->postcode,entry->population,entry->contact_name,entry->x,entry->y);
}

// Frees the string entries, watchtower entries and entire array
void FreeWts(wt_info_t **watchtowers, int *n) {
    int i;
    for (i=0;i<*n;i++) {
        wt_info_t *entry = watchtowers[i];
        free(entry->ID);
        free(entry->postcode);
        free(entry->contact_name);
        free(entry);
    }
    free(watchtowers);
}