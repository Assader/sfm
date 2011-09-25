/*
 * Обрабатывает нажатие Enter.
 * Если текущий элемент - директория, переходит в нее,
 *                        бинарник - выполняет в терминале,
 * иначе ищет в ftypes команду для этого типа файла,
 * если там такой нет, пробует открыть текстовым редактором.
 */

void kEnter(part *fPart, char *tEditor, char *term, int mRow, fts **ftypes, int numbOfFTypes){
    int i=0;
    char *tmp;
    struct stat fStat;

    if (fPart->f.numbOfLines){
        strcpy(fTmp, fPart->f.path);
        strcat(fTmp, fPart->f.files[fPart->w.currentLine]->d_name);
        stat(fTmp, &fStat);
        if (S_ISDIR(fStat.st_mode)){
            strcat(fPart->f.path, fPart->f.files[fPart->w.currentLine]->d_name);
            strcat(fPart->f.path, "/");
            fillList(fPart);
            setTop(&fPart->w, mRow, 1);
        }
        else if (fStat.st_mode & S_IXUSR){
            strcpy(sTmp, term);
            strcat(sTmp, " ");
            strcat(sTmp, fTmp);
            execInBkg(sTmp);
        }
        else if (S_ISREG(fStat.st_mode)){
            strcpy(sTmp, tEditor);
            tmp=&fTmp[strlen(fTmp)-1];
            while (((*tmp)!='.')&&(tmp != &fTmp[0]))
                --tmp;
            ++tmp;
            while(i<numbOfFTypes){
                if (!strcmp(ftypes[i]->filetype, tmp)){
                    strcpy(sTmp, ftypes[i]->cmd);
                    strcat(sTmp, " \'");
                    strcat(sTmp, fTmp);
                    strcat(sTmp, "\'");
                    break;
                }
                ++i;
            }
            execInBkg(sTmp);
        }
    }
}
