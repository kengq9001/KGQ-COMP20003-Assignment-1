// dcel_ops.c
// DCEL Operations

// Handles the following:
// - Creating a DCEL and its components
//       (vertices, faces, edges, half-edges i.e. hedges)
// - Constructing a polygon
// - Splitting a face by bisecting two edges
// - Checking if a point lies in a "clockwise" half-plane of two points
// - Freeing a DCEL and its components

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dcel_ops.h"

#define V_START_SIZE 4
#define F_START_SIZE 4
#define E_START_SIZE 4
#define H_START_SIZE 4

// Labelling the outside face simplifies things significantly
// And it is also counted in the formula V + F = E + 2 for planar graphs.
#define EXTERIOR_FACE -1

//==============================================================================
// The following 5 functions create a DCEL and its elements
//==============================================================================

// Returns pointer to a newly created DCEL
dcel_t *CreateDcel() {
    dcel_t *dcel;
    if ( (dcel = (dcel_t*)malloc(sizeof(dcel_t))) == NULL ) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }

    dcel->num_vertex = 0;
    dcel->size_vertex_list = V_START_SIZE;
    if ((dcel->vertex_list = (vertex_t**)malloc(sizeof(vertex_t*)*V_START_SIZE)) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }

    dcel->num_face = 0;
    dcel->size_face_list = F_START_SIZE;
    if ((dcel->face_list = (face_t**)malloc(sizeof(face_t*)*F_START_SIZE)) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }

    dcel->num_edge = 0;
    dcel->size_edge_list = E_START_SIZE;
    if ((dcel->edge_list = (edge_t**)malloc(sizeof(edge_t*)*E_START_SIZE)) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }

    return dcel;
}

void AddVertex(dcel_t *dcel, double x, double y) {
    int *n = &dcel->num_vertex;
    int *size = &dcel->size_vertex_list;
    // reallocate space if not enough
    if (*n == *size) {
        *size*=2;
        if ( (dcel->vertex_list = (vertex_t**)
        realloc(dcel->vertex_list, sizeof(vertex_t*)*(*size))) == NULL ) {
            printf("realloc() error\n");
            exit(EXIT_FAILURE);
        }
    }
    vertex_t *v;
    if ( (v = (vertex_t*)malloc(sizeof(vertex_t))) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    v->index = *n;
    v->x = x;
    v->y = y;
    dcel->vertex_list[*n] = v;
    *n+=1;
}

void AddFace(dcel_t *dcel) {
    int *n = &dcel->num_face;
    int *size = &dcel->size_face_list;
    // reallocate space if not enough
    if (*n == *size) {
        *size*=2;
        if ( (dcel->face_list = (face_t**)
        realloc(dcel->face_list, sizeof(face_t*)*(*size))) == NULL ) {
            printf("realloc() error\n");
            exit(EXIT_FAILURE);
        }
    }
    face_t *f;
    if ( (f = (face_t*)malloc(sizeof(face_t))) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    f->index = *n;
    f->hedge = NULL;
    dcel->face_list[*n] = f;
    *n+=1;
}

// Also adds 2 half-edges, v_s-->v_e having face f1 and v_e-->v_s having face f2
// This new edge is automatically linked to the first half edge.
// The two half-edges are automatically linked as twins
void AddEdge(dcel_t *dcel, int v_s, int v_e, int f1, int f2) {
    int *n = &dcel->num_edge;
    int *size = &dcel->size_edge_list;
    // reallocate space if not enough
    if (*n == *size) {
        *size*=2;
        if ( (dcel->edge_list = (edge_t**)
        realloc(dcel->edge_list, sizeof(edge_t*)*(*size))) == NULL ) {
            printf("realloc() error\n");
            exit(EXIT_FAILURE);
        }
    }
    edge_t *e;
    if ( (e = (edge_t*)malloc(sizeof(edge_t))) == NULL) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    e->index = *n;
    e->hedge = AddHedge(v_s, v_e, f1, *n);
    e->hedge->twin = AddHedge(v_e, v_s, f2, *n);
    e->hedge->twin->twin = e->hedge;
    dcel->edge_list[*n] = e;
    *n+=1;
}

hedge_t *AddHedge(int v_s, int v_e, int f, int e) {
    hedge_t *hedge;
    if ( (hedge = (hedge_t*)malloc(sizeof(hedge_t))) == NULL ) {
        printf("malloc() error\n");
        exit(EXIT_FAILURE);
    }
    hedge->v_start = v_s;
    hedge->v_end = v_e;
    hedge->face = f;
    hedge->edge = e;
    hedge->next = NULL;
    hedge->prev = NULL;
    hedge->twin = NULL;
    return hedge;
}

//==============================================================================
// Reads the file containing the vertices of the polygon
// and constructs a DCEL
//==============================================================================
void FirstPolygon(dcel_t *dcel, FILE *file) {
    // Add all vertices
    double X, Y;
    while (fscanf(file, "%lf %lf", &X, &Y) > 0) {
        AddVertex(dcel, X, Y);
    }

    // Add 1 face and all edges
    AddFace(dcel);
    int i;
    int n = dcel->num_vertex;
    for (i=0;i<n;i++) {
        AddEdge(dcel, i, (i+1)%n, 0, EXTERIOR_FACE);
    }

    // Link half edges, assuming that the polygon's vertices are oriented clockwise
    // %n as a neat trick to "wrap around" between indices 0 and n-1
    dcel->face_list[0]->hedge = dcel->edge_list[0]->hedge;
    for (i=0;i<n;i++) {
        dcel->edge_list[i]->hedge->next = dcel->edge_list[(i+1)%n]->hedge;
        dcel->edge_list[i]->hedge->prev = dcel->edge_list[(n+i-1)%n]->hedge;
        dcel->edge_list[i]->hedge->twin->next = dcel->edge_list[(n+i-1)%n]->hedge->twin;
        dcel->edge_list[i]->hedge->twin->prev = dcel->edge_list[(i+1)%n]->hedge->twin;
    }
}

//==============================================================================
// Bisects two edges, which adds 2 vertices + the edge which does the bisection
// + a face. Also makes appropriate changes to the half edges.
//==============================================================================
void SplitFace(dcel_t *dcel, int e1, int e2) {
    // First identify the face where the split is occuring.
    // Then make the edge point to the half-edge that is part of the face, 
    // rather than point to the half-edge that is not part of the face.
    // This ensures that the new edges are created in the correct position and order.

    hedge_t *h1 = dcel->edge_list[e1]->hedge;
    hedge_t *h2 = dcel->edge_list[e2]->hedge;

    // Temporary variables to help compute
    // the midpoint M of AB and midpoint N of CD
    // (AB and CD could be oriented incorrectly, but that does not matter)
    int e1A = h1->v_start;
    int e1B = h1->v_end;
    double Ax = dcel->vertex_list[e1A]->x;
    double Ay = dcel->vertex_list[e1A]->y;
    double Bx = dcel->vertex_list[e1B]->x;
    double By = dcel->vertex_list[e1B]->y;
    int e2A = h2->v_start;
    int e2B = h2->v_end;
    double Cx = dcel->vertex_list[e2A]->x;
    double Cy = dcel->vertex_list[e2A]->y;
    double Dx = dcel->vertex_list[e2B]->x;
    double Dy = dcel->vertex_list[e2B]->y;

    double Mx = (Ax + Bx) / 2;
    double My = (Ay + By) / 2;
    double Nx = (Cx + Dx) / 2;
    double Ny = (Cy + Dy) / 2;
    // create a temporary test point P for testing face
    double Px = (Mx + Nx) / 2;
    double Py = (My + Ny) / 2;
    if (!HalfPlane(dcel, h1->v_start, h1->v_end, Px, Py)) {
        dcel->edge_list[e1]->hedge = dcel->edge_list[e1]->hedge->twin;
    }
    if (!HalfPlane(dcel, h2->v_start, h2->v_end, Px, Py)) {
        dcel->edge_list[e2]->hedge = dcel->edge_list[e2]->hedge->twin;
    }

    // Now both edges point to half-edges which are in the face we are splitting

    // Now let first edge = A-->B, second edge = C-->D (clockwise order)
    // Add 2 vertices
    // Let this be M, midpoint of A-->B
    int m_i = dcel->num_vertex;
    AddVertex(dcel, Mx, My);
    // Let this be N, midpoint of C-->D
    int n_i = dcel->num_vertex;
    AddVertex(dcel, Nx, Ny);

    // Give names to edges
    hedge_t *AM = dcel->edge_list[e1]->hedge;
    hedge_t *ND = dcel->edge_list[e2]->hedge;

    // face indices
    int face_old = AM->face;
    int face_out1 = AM->twin->face;
    int face_out2 = ND->twin->face;
    int face_new = dcel->num_face;
    AddFace(dcel);

    // Add 3 edges

    // 1st edge
    // add M-->N
    AddEdge(dcel, m_i, n_i, face_old, face_new);
    hedge_t *MN = dcel->edge_list[dcel->num_edge-1]->hedge;
    dcel->face_list[face_old]->hedge = MN;
    dcel->face_list[face_new]->hedge = MN->twin;

    // Pre-processing for 2nd edge
    // change A-->B into A-->M
    e1B = AM->v_end;
    hedge_t *old_next_hedge = AM->next;
    hedge_t *old_prev_hedge_twin = AM->twin->prev;
    AM->v_end = m_i;
    AM->twin->v_start = m_i;
    AM->next = MN;
    MN->prev = AM;

    // 2nd edge
    // add M-->B
    AddEdge(dcel, m_i, e1B, face_new, face_out1);
    hedge_t *MB = dcel->edge_list[dcel->num_edge-1]->hedge;
    MB->next = old_next_hedge;
    old_next_hedge->prev = MB;
    MB->prev = MN->twin;
    MN->twin->next = MB;
    MB->twin->prev = old_prev_hedge_twin;
    old_prev_hedge_twin->next = MB->twin;
    MB->twin->next = AM->twin;
    AM->twin->prev = MB->twin;

    // Pre-processing for 3rd edge
    // change C-->D into N-->D
    e2A = ND->v_start;
    hedge_t *old_prev_hedge = dcel->edge_list[e2]->hedge->prev;
    hedge_t *old_next_hedge_twin = dcel->edge_list[e2]->hedge->twin->next;
    ND->v_start = n_i;
    ND->twin->v_end = n_i;
    ND->prev = MN;
    MN->next = ND;

    // 3rd edge
    // add C-->N
    AddEdge(dcel, e2A, n_i, face_new, face_out2);
    hedge_t *CN = dcel->edge_list[dcel->num_edge-1]->hedge;
    CN->prev = old_prev_hedge;
    old_prev_hedge->next = CN;
    CN->next = MN->twin;
    MN->twin->prev = CN;
    CN->twin->next = old_next_hedge_twin;
    old_next_hedge_twin->prev = CN->twin;
    CN->twin->prev = ND->twin;
    ND->twin->next = CN->twin;

    // update faces
    hedge_t *h = MB;
    while (h != CN) {
        h->face = face_new;
        h = h->next;
    }
}

// Returns 1 if P is in the same half plane as the half edge A-->B, otherwise returns 0
// By right hand rule, this is true if (A-->P) X (A-->B) has positive orientation
// i.e. the quantity x1y2 - x2y1 is positive
int HalfPlane(dcel_t *dcel, int v1, int v2, double Px, double Py) {
    vertex_t *A = dcel->vertex_list[v1];
    vertex_t *B = dcel->vertex_list[v2];
    return ((Px - A->x)*(B->y - A->y) > (B->x - A->x)*(Py - A->y));

    // Remarks:
    // For a more general usage of this function, I would instead
    // return 1 if P is in the "clockwise" half plane of AB
    // return 0 if P lies on AB
    // return -1 if P is in the "anticlockwise" half plane of AB
    // This will suffice for now
}

// Prints out DCEL for debugging
void PrintDcel(dcel_t *dcel) {
    int i;
    printf("Number of vertices: %d\n", dcel->num_vertex);
    for (i=0;i<dcel->num_vertex;i++) {
        vertex_t *v = dcel->vertex_list[i];
        printf("Index: %d, X: %f, Y: %f\n", v->index, v->x, v->y);
    }
    printf("Number of faces: %d\n", dcel->num_face);
    for (i=0;i<dcel->num_face;i++) {
        face_t *f = dcel->face_list[i];
        printf("Index: %d, Hedge: %p\n", f->index, f->hedge);
    }
    printf("Number of edges: %d\n", dcel->num_edge);
    for (i=0;i<dcel->num_edge;i++) {
        edge_t *e = dcel->edge_list[i];
        hedge_t *h = e->hedge;
        printf("Index: %d, Hedge 1: %p, Hedge 2: %p\n", e->index, h, h->twin);
        printf("\tHedge 1: V: %d->%d, F: %d, E: %d, N: %p, P: %p, T: %p\n",
        h->v_start,h->v_end,h->face,h->edge,h->next,h->prev,h->twin);
        h = h->twin;
        printf("\tHedge 2: V: %d->%d, F: %d, E: %d, N: %p, P: %p, T: %p\n",
        h->v_start,h->v_end,h->face,h->edge,h->next,h->prev,h->twin);
    }
}

// Free the DCEL components and the DCEL
void FreeDcel(dcel_t *dcel) {
    int i;

    for (i=0;i<dcel->num_vertex;i++) {
        free(dcel->vertex_list[i]);
    }
    free(dcel->vertex_list);

    for (i=0;i<dcel->num_face;i++) {
        free(dcel->face_list[i]);
    }
    free(dcel->face_list);

    for (i=0;i<dcel->num_edge;i++) {
        free(dcel->edge_list[i]->hedge->twin);
        free(dcel->edge_list[i]->hedge);
        free(dcel->edge_list[i]);
    }
    free(dcel->edge_list);

    free(dcel);
}