/*
 * Принимает указатель на файловую переменную, связанную с конфигурационный файлом, открытым для записи,
 *  и массив строк с названиями действий, которые нужно связать с клавишами.
 * Кроме действий из actions запрашивает название текстового редактора и эмулятора терминала.
 * Устанавливает локаль по $LANG.
 */

void setKeys(FILE *f, const char **actions){
    int i=0, ch;

    echo();
    clear();
    printw("Enter name of your text editor: ");
    scanw("%[^\n]", fTmp);
    fprintf(f, "[Global]\nText editor = %s ;\n", fTmp);
    clear();
    printw("Enter name of your terminal emulator\nwith flags to execute: ");
    scanw("%[^\n]", fTmp);
    fprintf(f, "Terminal emulator = %s ;\nLocale = %s ;\n[Binds]\n", fTmp, getenv("LANG"));
    noecho();
    while (i<ACTIONS){
        clear();
        printw("Enter symbol to action \"%s\": ", actions[i]);
        ch=getch();
        fprintf(f, "%s = %c ;\n", actions[i++], ch);
    }
    fprintf(f, "[uBinds]\n\n[Filetypes]\n");
}
