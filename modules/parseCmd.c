/*
 * Заменяет вхождения %cf, %cF, %af в cmd на путь к текущей директории, название текущего файла и
 *  путь к неактивной директории соответственно.
 * %rc, %ra - обновляют содержимое директории в активном и неактивном окне соответствено.
 */

void parseCmd(part *fPart, part *sPart, char *cmd){
    char *outCmd = (char *) malloc(sizeof(char)*1024),
         *tmp;
    part *rf=NULL;

    strcpy(fTmp, cmd);
    strcpy(outCmd, "");
    tmp = strtok(fTmp, "%");
    while (tmp){
        if (tmp[0]=='U'){
            ++tmp;
            ask(tmp, sTmp);
            if (!strcmp(sTmp, ""))
                return;
            strcat(outCmd, sTmp);
        }
        else if (((tmp[0]=='c')||(tmp[0]=='a'))&&((tmp[1]=='f')||(tmp[1]=='F'))){
            if ((tmp[0]=='c')&&(tmp[1]=='f'))
                strcat(outCmd, fPart->f.path);
            else if ((tmp[0]=='c')&&(tmp[1]=='F'))
                strcat(outCmd, fPart->f.files[fPart->w.currentLine]->d_name);
            else if (tmp[0]=='a'&&(tmp[1]=='f'))
                strcat(outCmd, sPart->f.path);
            tmp+=2;
            tmp ? strcat(outCmd, tmp) : 0;
        }
        else if ((tmp[0]=='r')&&((tmp[1]=='c')||(tmp[1]=='a')))
            rf = (tmp[1]=='c')? fPart : sPart;
        else
            strcat(outCmd, tmp);
        tmp = strtok(NULL, "%");    
    }
    system(outCmd);
    if (rf)
        fillList(rf);

    free(tmp);
    free(outCmd);
}
