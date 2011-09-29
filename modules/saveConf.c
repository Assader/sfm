/*
 * Сохраняет путь левой и правой части окна, настройки видимости.
 */

void saveConf(part *fPart, part *sPart){
    sprintf(fTmp, "%s/.config/sfm/tmp", getenv("HOME"));
    FILE *f = fopen(fTmp, "w");
    fprintf(f, "[Main]\nlPath = %s ;\nrPath = %s ;\nlHid = %d ;\nrHid = %d ;\n",
               fPart->f.path, sPart->f.path, fPart->f.showHidden, sPart->f.showHidden);
    fclose(f);
}