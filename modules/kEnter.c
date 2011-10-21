/*
 * Обрабатывает нажатие Enter.
 * Если текущий элемент - директория, переходит в нее,
 *                        бинарник - выполняет в терминале,
 * иначе - ищет в ftypes команду для этого типа файла,
 * если там такой нет, пробует открыть текстовым редактором.
 */

void kEnter(part *fPart, char *tEditor, char *term, int mRow, fts **ftypes, int numbOfFTypes){
    int i=0;
    char *tmp;
    struct stat fStat;

    if (fPart->f.numbOfLines){
        sprintf(fTmp, "%s%s", fPart->f.path, fPart->f.files[fPart->w.currentLine]->d_name);
        stat(fTmp, &fStat);
        if (S_ISDIR(fStat.st_mode)){
            strcat(fPart->f.path, fPart->f.files[fPart->w.currentLine]->d_name);
            strcat(fPart->f.path, "/");
            fillList(fPart);
            setTop(&fPart->w, mRow, 1);
        }
        else if (S_ISREG(fStat.st_mode)){
            if (fStat.st_mode & S_IXUSR)
                sprintf(sTmp, "%s \'%s\'", term, fTmp);
            else{
                sprintf(sTmp, "%s \'%s\'", tEditor, fTmp);
                tmp=&fTmp[strlen(fTmp)-1];
                while (((*(tmp-1))!='.')&&(tmp != &fTmp[0]))
                    --tmp;
                for(;i<numbOfFTypes;i++)
                    if (!strcmp(ftypes[i]->filetype, tmp)){
                        sprintf(sTmp, "%s \'%s\'", ftypes[i]->cmd, fTmp);
                        break;
                    }
            }
                execInBkg(sTmp);
        }
    }
}
