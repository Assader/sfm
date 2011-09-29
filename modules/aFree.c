/*
 * Освобождает память, выделенную под len элеметов ptr.
 */

void aFree(void **ptr, int len){
    while(len--)
        free(ptr[len]);
    free(ptr);
}
