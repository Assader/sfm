/*
 * Проверяет, является ли строка путем к директории.
 */

int isValidPath(char *path){
    struct stat fStat;

    stat(path, &fStat);
    return S_ISDIR(fStat.st_mode);
}
