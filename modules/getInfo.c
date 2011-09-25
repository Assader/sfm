/*
 * Выводит информацию по текущему элементу:
 *  полное имя
 *  размер
 *  права доступа
 *  владельца
 *  2do - даты создания. изменения.
 */

void getInfo(part *fPart, wnd *fWind, int mCol, int mRow){
    const short int octarray[9] = {0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001};
    const char *rwx="rwx";
    int tY=1, i;
    struct stat fStat;

    wclear(fWind->win);
    box(fWind->win, 0, 0);
    strcpy(fTmp, fPart->f.path);
    strcat(fTmp, fPart->f.files[fPart->w.currentLine]->d_name);
    stat(fTmp, &fStat);
    mvwprintw(fWind->win, tY, 1, "Name: %s", fPart->f.files[fPart->w.currentLine]->d_name);
    tY += (strlen(fPart->f.files[fPart->w.currentLine]->d_name)+6)/(mCol/2) + 1;
    mvwprintw(fWind->win, tY++, 1, "Size: %s", parseSize(fStat.st_size));
    mvwprintw(fWind->win, tY++, 1, "Access mode: ");
    for (i=0;i<9;i++){
        if (fStat.st_mode & octarray[i])
            wprintw(fWind->win, "%c", rwx[i%3]);
        else
            wprintw(fWind->win, "-");
        ((i+1)%3==0) ? wprintw(fWind->win, " ") : 0;
    }
    wprintw(fWind->win, "(%o)", (fStat.st_mode & S_IRWXU)+(fStat.st_mode & S_IRWXG)+(fStat.st_mode & S_IRWXO));
    mvwprintw(fWind->win, tY++, 1, "Owner: ");
    sprintf(fTmp, "getent passwd %d > /tmp/sfm-Owner", fStat.st_uid);
    system(fTmp);
    FILE *f = fopen("/tmp/sfm-Owner", "r");
    fscanf(f, "%[^:]", fTmp);
    fclose(f);
    wprintw(fWind->win, "%s", fTmp);
    move(mRow-1, 0);
    wrefresh(fWind->win);
    getch();
}
