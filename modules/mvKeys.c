/*
 * Сдвигает currentLine на 1 вверх.
 */

void kUp(wnd *fWind, int numbOfLines, int mRow){
    --fWind->currentLine;
    if (fWind->currentLine<0){
        fWind->currentLine = numbOfLines-1;
        if (numbOfLines>mRow-4)
            fWind->top = fWind->currentLine-(mRow-5);
        else
            fWind->top = 0;
    }
    if (fWind->currentLine<fWind->top)
        --fWind->top;
}

/*
 * Сдвигает currentLine на 1 вниз.
 */

void kDown(wnd *fWind, int numbOfLines, int mRow){
    ++fWind->currentLine;
    if (fWind->currentLine>numbOfLines-1){
        fWind->currentLine = 0;
        fWind->top = 0;
    }
    if (fWind->currentLine>fWind->top+(mRow-5))
        ++fWind->top;
}

/*
 * Обнуляет currentLine.
 */

void kHome(wnd *fWind){
    fWind->currentLine = 0;
    fWind->top = 0;
}

/*
 * Устанавливает currentLine на последний элемент.
 */

void kEnd(part *fPart, int mRow){
    fPart->w.currentLine = fPart->f.numbOfLines-1;
    setTop(&fPart->w, mRow, 0);
}

/*
 * Поднимает currentLine на "страницу" (mRow-4 элемента).
 */

void kPUp(wnd *fWind, int mRow){
    if ((fWind->currentLine - (mRow-4))>0){
        fWind->currentLine = fWind->currentLine - (mRow-4);
        setTop(fWind, mRow, 0);
        }
    else
        kHome(fWind);
}

/*
 * Опускает currentLine на "страницу" (mRow-4 элемента).
 */

void kPDown(part *fPart, int mRow){
    if ((fPart->w.currentLine + (mRow-4))<fPart->f.numbOfLines){
        fPart->w.currentLine = fPart->w.currentLine + (mRow-4);
        setTop(&fPart->w, mRow, 0);
    }
    else
        kEnd(fPart, mRow);
}
