/*
 * Принимает ссылку на ссылку на ссылку (сслыку на массив ссылок) типа bnd, ссылку на numbOfBinds и
 *  ссылку на словарь ini, связанный с конфигурационным файлом.
 * Находит по словарю значения, указанные в секции uBinds конф. файла, запиывает пару "клавиша - команда"
 *  в ubinds и увеличивает numbOfBinds на 1.
 */

void getUserBinds(bnd ***ubinds, int *numbOfBinds, dictionary *ini){
    const char *symbols = "qwertyuiopasdfghjklzxcvbnm";
    *ubinds = (bnd **) malloc(sizeof(bnd *));
    int i=0;

    while (i<strlen(symbols)){
        sprintf(fTmp, "ubinds:%c", symbols[i]);
        strcpy(fTmp, iniparser_getstring(ini, fTmp, ""));
        if (fTmp[0]!='\0'){
            ((*numbOfBinds)>0) ? *ubinds = (bnd **) realloc(*ubinds, sizeof(bnd *)*((*numbOfBinds)+1)) : 0;
            (*ubinds)[*numbOfBinds] = (bnd *) malloc(sizeof(bnd));
            (*ubinds)[*numbOfBinds]->key = symbols[i];
            (*ubinds)[*numbOfBinds]->cmd = (char *) malloc((size_t)strlen(fTmp)+1);
            strcpy((*ubinds)[*numbOfBinds]->cmd, fTmp);
            ++(*numbOfBinds);
        }
        ++i;
    }
}
