/*
 * Абстракция. Надстройка над ф-циями для прорисовки интерфейса
 */

void process(part *fPart, int mRow, int mCol, int showNfo){
    wclear(fPart->w.win);
    box(fPart->w.win, 0, 0);
    draw(fPart, mRow, mCol);
    if (showNfo)
        nfo(fPart, mCol);
    wrefresh(fPart->w.win);
}
