/*
 * Если под fPart->f.files была выделена память, освобождает ее.
 * Считывает директорию, путь к которой находится в fPart->f.path,
 *  помещает результат в fPart->f.files,
 *  использует селектор slt, если fPart->f.showHidden = 0,
 *  сортирует в алфавитном порядке.
 */

void fillList(part *fPart){
    if (fPart->f.numbOfLines)
        aFree((void **)fPart->f.files, fPart->f.numbOfLines);
    fPart->f.numbOfLines = scandir(fPart->f.path, &fPart->f.files, (fPart->f.showHidden)?0:slt, alphasort);
}
