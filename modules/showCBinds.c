/*
 * Выводит список "клавиша - бинд" из ubinds.
 */

void showCBinds(part *fPart, int mRow, bnd **ubinds, int numbOfBinds){
    int i=0, tY, key=0;

    while (1){
        tY=0;
        wclear(fPart->w.win);
        box(fPart->w.win, 0, 0);
        while ((tY<mRow-4)&&(i+tY<numbOfBinds)){
            mvwprintw(fPart->w.win, tY+1, 1, "%c = %s", ubinds[i+tY]->key, ubinds[i+tY]->cmd);
            ++tY;
        }
        move(mRow-1, 0);
        wrefresh(fPart->w.win);
        key = getch();
        if (key==KEY_UP)
            --i;
        else if (key==KEY_DOWN)
            ++i;
        else break;
        if ((i<0)||(i+tY>numbOfBinds))
            i=0;
    }
}
