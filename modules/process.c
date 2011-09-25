/*
 * Абстракция. Надстройка над ф-циями для прорисовки интерфейса
 */

void process(part *fPart, int mRow, int mCol, int showNfo){
    wclear(fPart->w.win);
    box(fPart->w.win, 0, 0);
    draw(fPart, mRow, mCol);
    showNfo ? nfo(fPart, mCol) : 0;
    wrefresh(fPart->w.win);
}
