// hedge meaning half-edge
typedef struct hedge hedge_t;

struct hedge {
    int v_start;
    int v_end;
    int face;
    int edge;
    hedge_t *next;
    hedge_t *prev;
    hedge_t *twin;
};

typedef struct {
    int index;
    double x;
    double y;
} vertex_t;

typedef struct {
    int index;
    hedge_t *hedge;
} face_t;

typedef struct {
    int index;
    hedge_t *hedge;
} edge_t;

typedef struct {
    int num_vertex;
    int size_vertex_list;
    vertex_t **vertex_list;
    int num_face;
    int size_face_list;
    face_t **face_list;
    int num_edge;
    int size_edge_list;
    edge_t **edge_list;
} dcel_t;

dcel_t *CreateDcel();

void AddVertex(dcel_t *dcel, double x, double y);

void AddFace(dcel_t *dcel);

void AddEdge(dcel_t *dcel, int v_s, int v_e, int f1, int f2);

hedge_t *AddHedge(int v_s, int v_e, int f, int e);

void FirstPolygon(dcel_t *dcel, FILE *file);

void SplitFace(dcel_t *dcel, int e1, int e2);

int HalfPlane(dcel_t *dcel, int v1, int v2, double P_x, double P_y);

void PrintDcel(dcel_t *dcel);

void FreeDcel(dcel_t *dcel);