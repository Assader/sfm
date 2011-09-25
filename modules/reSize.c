/*
 * 
 */

void reSize(wnd *fWind, wnd *sWind, int *mRow, int *mCol){
    int tmX, tmY;

    getmaxyx(stdscr, tmY, tmX);
    if ((*mRow!=tmY)||(*mCol!=tmX)){
        delwin(fWind->win);
        delwin(sWind->win);
        wclear(stdscr);
        fWind->win = newwin(tmY-2, tmX/2, 1, 0);
        sWind->win = newwin(tmY-2, tmX/2, 1, tmX/2);
        *mRow=tmY;
        *mCol=tmX;
    }
}
