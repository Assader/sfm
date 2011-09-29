/*
 * Отрисовывает в fPart->w.win содержимое fPart->f.files, начиная с top, заканчивая top + mRow - 4.
 * Если элемент является директорией, в конец дописывается '/'
 *                       исполняемым файлом     -          '*'
 *                       ссылкой                -          '>'
 *                       символьным устройством -          '@'
 *                       блочным устройством    -          '#'
 * Название элемента обрезается до mCol/2-3. mCol/2 - половина всего (1 окно). -2 для бордеров и -1 для идентификатора (дир, ссылка, etc).
 */

void draw(part *fPart, int mRow, int mCol){
    int tY=0;
    struct stat fStat;

    while ((tY<mRow-4)&&(tY<fPart->f.numbOfLines)){
        mvwprintw(fPart->w.win, tY+1, 1, "%.*s", mCol/2-3, fPart->f.files[fPart->w.top + tY]->d_name);
        sprintf(fTmp, "%s%s", fPart->f.path, fPart->f.files[fPart->w.top + tY++]->d_name);
        stat(fTmp, &fStat);
        if (S_ISDIR(fStat.st_mode))
            wprintw(fPart->w.win, "/");
        else if (fStat.st_mode & S_IXUSR)
            wprintw(fPart->w.win, "*");
        else if (S_ISLNK(fStat.st_mode))
            wprintw(fPart->w.win, ">");
        else if (S_ISCHR(fStat.st_mode))
            wprintw(fPart->w.win, "@");
        else if (S_ISBLK(fStat.st_mode))
            wprintw(fPart->w.win, "#");
    }
}
