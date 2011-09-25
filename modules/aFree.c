/*
 * Освобождает память, выделенную под ptr длинной len.
 *  Трактуется как "массив ссылок".
 */

void aFree(void **ptr, int len){
    while(len--)
        free(ptr[len]);
    free(ptr);
}
