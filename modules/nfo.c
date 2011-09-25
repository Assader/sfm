/*
 * Выводит строку состояния - путь до текущей директории, текущее положение и кол-во элементов в директории.
 */

void nfo(part *fPart, int mCol){
    char *rd;

    mvwchgat(fPart->w.win, fPart->w.currentLine-fPart->w.top+1, 1, mCol/2-2, A_REVERSE, 0, NULL);
    if (strlen(fPart->f.path)<(mCol-15))
        mvprintw(0, 0, "%s", fPart->f.path);
    else{
        rd=&fPart->f.path[strlen(fPart->f.path)-mCol+17];
        mvprintw(0, 0, "..");
        printw("%s", rd);
    }
    mvprintw(0, mCol-15, "|%d/%d", fPart->w.currentLine+1, fPart->f.numbOfLines);
}
