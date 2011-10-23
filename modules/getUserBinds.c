/*
 * Принимает ссылку на ссылку на ссылку (сслыку на массив ссылок) типа bnd, ссылку на numbOfBinds и
 *  ссылку на словарь ini, связанный с конфигурационным файлом.
 * Находит по словарю значения, указанные в секции uBinds конф. файла, запиывает пару "клавиша - команда"
 *  в ubinds и увеличивает numbOfBinds на 1.
 */

void getUserBinds(bnd ***ubinds, int *numbOfBinds, dictionary *ini){
    const char *symbols = "qwertyuiopasdfghjklzxcvbnm";
    int i;
    *ubinds = (bnd **) malloc(sizeof(bnd *));

    for(i=0;i<26;i++){
        sprintf(fTmp, "ubinds:%c", symbols[i]);
        strcpy(fTmp, iniparser_getstring(ini, fTmp, ""));
        if (fTmp[0]!='\0'){
            if ((*numbOfBinds)>0)
                *ubinds = (bnd **) realloc(*ubinds, sizeof(bnd *)*((*numbOfBinds)+1));
            (*ubinds)[*numbOfBinds] = (bnd *) malloc(sizeof(bnd));
            (*ubinds)[*numbOfBinds]->key = symbols[i];
            (*ubinds)[*numbOfBinds]->cmd = (char *) malloc((size_t)strlen(fTmp)+1);
            strcpy((*ubinds)[*numbOfBinds]->cmd, fTmp);
            ++(*numbOfBinds);
        }
    }
}
