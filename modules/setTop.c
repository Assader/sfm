/*
 * Устанавливает top текущего окна в зависимости от currentLine и mRow (максимального кол-ва строк окна).
 * Если mkSt != 0, обнуляет текущее положение и top.
 */

void setTop(wnd *fWind, int mRow, int mkSt){
    if (mkSt)
        fWind->currentLine=0;
    if (fWind->currentLine>mRow-5)
        fWind->top = fWind->currentLine-(mRow-5);
    else
        fWind->top = 0;
}
