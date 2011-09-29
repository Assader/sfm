/*
 * Получает на вход кол-во байт и возвращает строку с числом, кратным исходному, и приставкой.
 */

char *parseSize(long int s){
    const char *sizes[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB"};
    int i=0;
    double sz = (double) s;

    while (sz>1024){
        sz /= 1024;
        ++i;
    }
    sprintf(fTmp, "%.2lf %s", sz, sizes[i]);

    return fTmp;
}
