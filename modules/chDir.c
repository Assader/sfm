/*
 * Запрашивает название директории и переходит в нее.
 * Если введенная строка начинается с '/', то путь рассматривается как абсолютный.
 * Гарантирует, что Part->f.path[ strlen( Part->f.path ) - 1 ] == '/'.
 */

void chDir(fl *fFold, int mRow){
    struct stat fStat;

    ask("Name of folder", fTmp);
    if (strcmp(fTmp, "")){
        if (fTmp[strlen(fTmp)-1]!='/')
            strcat(fTmp, "/");
        if (fTmp[0]=='/')
            strcpy(sTmp, fTmp);
        else
            sprintf(sTmp, "%s%s", fFold->path, fTmp);
        stat(sTmp, &fStat);
        if (S_ISDIR(fStat.st_mode))
            strcpy(fFold->path, sTmp);
        else {
            mvchgat(mRow-1, 0, -1, A_INVIS, 0, NULL);
            printw("Wrong name of folder");
            getch();
        }
    }
}
