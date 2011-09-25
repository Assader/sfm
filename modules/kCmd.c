/*
 * Запрашивает команду на выполнение, если в ней найден символ '%', отправляет в parseCmd, иначе просто выполняет.
 */

void kCmd(part *fPart, part *sPart){
    ask("Command", fTmp);
    if (strcmp(fTmp, "")){
        if (strchr(fTmp, '%'))
            parseCmd(fPart, sPart, fTmp);
        else
            system(fTmp);
    }
}
