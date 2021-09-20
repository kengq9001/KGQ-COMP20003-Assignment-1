// Ken Gene Quah
// COMP20003 Assignment 1

// main.c
// Passes the files to the necessary functions/operations
// and creates the desired output.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "wt_ops.h"
#include "dcel_ops.h"

void CreateOutput(FILE *file, dcel_t *dcel, wt_info_t **wts, int n);

int main(int argc, char **argv) {

    if (argc<4) {
        printf("Insufficient input\n");
        return 0;
    }

    // Obtain watchtower data and no. of watchtowers
    FILE *file;
    if ((file = fopen(argv[1], "r")) == NULL) {
        printf("File 1 not found\n");
        return 0;
    }
    wt_info_t **watchtowers;
    int watchtowers_num = 0;
    watchtowers = ReadWtInfo(file, &watchtowers_num);
    fclose(file);

    // Create DCEL with initial polygon
    if ((file = fopen(argv[2], "r")) == NULL) {
        printf("File 2 not found\n");
        return 0;
    }
    dcel_t *DCEL;
    DCEL = CreateDcel();
    FirstPolygon(DCEL, file);

    // Read in and perform splits
    int a,b;
    while (scanf("%d %d", &a, &b) == 2) {
        SplitFace(DCEL, a, b);
    }
    fclose(file);

    // Output and free memory
    file = fopen(argv[3], "w");
    CreateOutput(file, DCEL, watchtowers, watchtowers_num);
    fclose(file);

    FreeDcel(DCEL);
    FreeWts(watchtowers, &watchtowers_num);
    return 0;

}
// For each face, print out the watchtowers which belong to it and 
// add up the populations.
void CreateOutput(FILE *file, dcel_t *dcel, wt_info_t **wts, int n) {

    int w, f, passed;

    // half_planes is a temporary array, half_planes[w] = 0 when
    // the wth watchtower fails a half plane check
    int *half_planes = (int*)malloc(sizeof(int)*n);

    int *face_populations = (int*)malloc(sizeof(int)*dcel->num_face);

    for (f=0;f<dcel->num_face;f++) {

        // initialise possibilities
        for (w=0;w<n;w++) {
            half_planes[w] = 1;
        }
        face_populations[f] = 0;

        // eliminate possibilities
        hedge_t *hedge = dcel->face_list[f]->hedge;
        passed = 0;
        while (hedge != dcel->face_list[f]->hedge || !passed) {
            passed = 1;
            for (w=0;w<n;w++) {
                if (!HalfPlane(dcel, hedge->v_start, hedge->v_end,
                    wts[w]->x, wts[w]->y)) {
                        half_planes[w] = 0;
                    }
            }
            hedge = hedge->next;
        }
        
        // sum remaining possibilities
        fprintf(file, "%d\n", f);
        for (w=0;w<n;w++) {
            if (half_planes[w]) {
                PrintWtInfo(file, wts[w]);
                face_populations[f] += wts[w]->population;
            }
        }
    }

    // Print populations and free temporary arrays
    for (f=0;f<dcel->num_face;f++) {
        fprintf(file, "Face %d population served: %d\n", f, face_populations[f]);
    }
    free(half_planes);
    free(face_populations);

}