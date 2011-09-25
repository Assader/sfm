/*
 * Запрашивает название директории и переходит в нее.
 * Если введенная строка начинается с '/', то путь рассматривается как абсолютный.
 * Гарантирует, что Part->f.path[ strlen( Part->f.path ) - 1 ] == '/'.
 */

void chDir(fl *fFold){
    struct stat fStat;

    ask("Name of folder", fTmp);
        if (strcmp(fTmp, "")){
            if (fTmp[strlen(fTmp)-1]!='/')
                strcat(fTmp, "/");
            if (fTmp[0]=='/'){
                stat(fTmp, &fStat);
                if (S_ISDIR(fStat.st_mode))
                    strcpy(fFold->path, fTmp);
                else {
                    printw("Wrong name\t\t");
                    getch();
                }
            }
            else{
                strcpy(sTmp, fFold->path);
                strcat(sTmp, fTmp);
                stat(sTmp, &fStat);
                if (S_ISDIR(fStat.st_mode))
                    strcat(fFold->path, fTmp);
                else {
                    printw("Wrong name\t\t");
                    getch();
                }
            }
        }
}
