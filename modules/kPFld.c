/*
 * Переходит в верхнюю директорию.
 *  = Обрезает путь по последнему '/'.
 */

void kPFld(fl *fFold){
    int i;

    for(i=strlen(fFold->path)-2;i>=0;i--)
        if (fFold->path[i]=='/'){
            fFold->path[i+1]='\0';
            break;
        }
}
