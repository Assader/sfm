/*
 * Запрашивает строку для перехода и переходит на нее.
 * Если она < 0 или > numbOfLines, обнуляет currentLine.
 */

void goToLine(part *fPart, int mRow){
    echo();
    printw("New position: ");
    scanw("%d", &fPart->w.currentLine);
    noecho();
    if ((--fPart->w.currentLine<0)||(fPart->w.currentLine>fPart->f.numbOfLines)){
        fPart->w.top = 0;
        fPart->w.currentLine = 0;
    }
    setTop(&fPart->w, mRow, 0);
}
