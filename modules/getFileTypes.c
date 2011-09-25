/*
 * Принимает ссылку на ссылку на ссылку (сслыку на массив ссылок) типа fts, ссылку на numbOfFTypes и
 *  ссылку на словарь ini, связанный с конфигурационным файлом.
 * Находит по словарю значения, указанные в секции Filetypes конф. файла, запиывает пару "расширение файла - программа"
 *  в ftypes и увеличивает numbOffTypes на 1.
 */

void getFileTypes(fts ***ftypes, int *numbOfFTypes, dictionary *ini){
    char *tmp;
    int i;
    *ftypes = (fts **) malloc(sizeof(fts *));

    strcpy(fTmp, iniparser_getstring(ini, "filetypes:types", NULL));
    tmp = strtok(fTmp, " ");
    while (tmp){
        i=0;
        sprintf(sTmp, "filetypes:%s", tmp);
        strcpy(sTmp, iniparser_getstring(ini, sTmp, NULL));
        ((*numbOfFTypes)>0) ? *ftypes = (fts **) realloc(*ftypes, sizeof(fts *)*((*numbOfFTypes)+1)) : 0;
        (*ftypes)[*numbOfFTypes] = (fts *) malloc(sizeof(fts));
        (*ftypes)[*numbOfFTypes]->filetype = (char *) malloc(sizeof(tmp)+1);
        (*ftypes)[*numbOfFTypes]->cmd = (char *) malloc(sizeof(sTmp)+1);
        strcpy((*ftypes)[*numbOfFTypes]->filetype, tmp);
        while (i<(*numbOfFTypes)){
            if (!strcmp((*ftypes)[i]->cmd, sTmp)){
                (*ftypes)[*numbOfFTypes]->cmd = (*ftypes)[i]->cmd;
                i=0;
                break;
            }
            ++i;
        }
        (i||(!(*numbOfFTypes))) ? strcpy((*ftypes)[*numbOfFTypes]->cmd, sTmp) : 0;
        ++(*numbOfFTypes);
        tmp = strtok(NULL, " ");    
    }
}
