/*
 * Селектор скрытых файлов.
 */

int slt(const struct dirent *d){
     return strncmp(d->d_name, ".", 1);
}
