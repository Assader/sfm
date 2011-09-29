/*
 * Считывает символы до первого непечатного, каждый раз устанавливая currentLine на элемент, максимально соответствующий
 *  введенным символам.
 */

void kSearch(part *fPart, int mRow, int mCol){
    int i=0, key, pos=0;

    echo();
    printw("/");
    while (isprint((key=getch()))){
            sTmp[i++] = key;
            sTmp[i] = '\0';
        for(;pos<fPart->f.numbOfLines;pos++)
            if (!strncmp(fPart->f.files[pos]->d_name, sTmp, i)){
                fPart->w.currentLine = pos;
                setTop(&fPart->w, mRow, 0);
                process(fPart, mRow, mCol, 1);
                move(mRow-1, i+1);
                break;
            }
    }
    noecho();
}
