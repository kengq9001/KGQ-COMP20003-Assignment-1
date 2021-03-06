typedef struct {
    char *ID;
    char *postcode;
    int population;
    char *contact_name;
    double x;
    double y;
} wt_info_t;

// realloc usage idea from https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/realloc.c
wt_info_t **ReadWtInfo(FILE *file, int *n);

// strtok usage idea from https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
wt_info_t *SplitLine(char *line);

char *OneString(char *line);

void PrintWtInfo(FILE *file, wt_info_t *entry);

void FreeWts(wt_info_t **watchtowers, int *n);