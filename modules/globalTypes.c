typedef struct{
    WINDOW *win;
    int currentLine;
    int top;
} wnd;

typedef struct{
    char *path;
    struct dirent **files;
    int numbOfLines;
    int showHidden;
} fl;

typedef struct{
    wnd w;
    fl f;
} part;

typedef struct{
    char key;
    char *cmd;
} bnd;

typedef struct{
    char *filetype;
    char *cmd;
    int lvl;
} fts;
