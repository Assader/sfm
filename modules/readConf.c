/*
 * Считывает конфигурационный файл, при его отсутствии создает его, предлагает сразу настроить или использовать стд натсройки.
 * Считывается:
 *      путь левой части окна
 *      путь правой части окна
 *      настройки видимости
 *      клавиши для стд действий
 *      эмулятор терминала
 *      текстовый редактор
 *      локаль
 *      uBinds
 *      fTypes
 */

#include "getUserBinds.c"
#include "getFileTypes.c"

void readConf(part *fPart, part *sPart, char *tEditor, char *term, int *keys, char *loc, bnd ***ubinds, int *numbOfBinds,
                                                                                         fts ***ftypes, int *numbOfFTypes){
    const char *actions[ACTIONS] = {"Another window", "Quit", "Hidden mode", "Same folder", "Change folder", "Parent folder",
                                    "Go to line", "Info", "Execute command", "Search"};
    int i=0;
    FILE *f;
    dictionary *ini;

    initscr();
    sprintf(fTmp, "%s/.config/sfm/tmp", getenv("HOME"));
    if (!(f = fopen(fTmp, "r"))){
        system("mkdir -p ~/.config/sfm/");
        f = fopen(fTmp, "w");
        fprintf(f, "[Main]\nlPath = / ;\nrPath = / ;\nlHid = 0 ;\nrHid = 0 ;\n");
    }
    fclose(f);
    ini = iniparser_load(fTmp);
    strcpy(fPart->f.path, iniparser_getstring(ini, "main:lpath", "/"));
    strcpy(sPart->f.path, iniparser_getstring(ini, "main:rpath", "/"));
    fPart->f.showHidden = iniparser_getint(ini, "main:lhid", 0);
    sPart->f.showHidden = iniparser_getint(ini, "main:rhid", 0);
    iniparser_freedict(ini);
    sprintf(fTmp, "%s/.config/sfm/sfm.conf", getenv("HOME"));
    if (!(f = fopen(fTmp, "r"))){
        printw("Main .conf file not found. (C)onfigure now or use (d)efault?");
        i = getch();
        f = fopen(fTmp, "w");
        if (i=='d')
            fprintf(f,"[Global]\nText editor = gedit ;\nTerminal emulator = xterm ;\nLocale = en_US.UTF-8 ;\n"
                      "[Binds]\nAnother window = a ;\nQuit = q ;\nHidden mode = h ;\nSame folder = s ;\n"
                      "Change folder = c ;\nParent folder = u ;\nGo to line = g ;\nInfo = i ;\nExecute command = k ;\n"
                      "Search = / ;\n[uBinds]\n\n[Filetypes]\nTypes = ;\n\n");
        else
            setKeys(f, actions);
        clear();
        printw("Config saved to ~/.config/sfm/sfm.conf\nYou can set user-binds and filetypes in.\nPress any key");
        getch();
        sprintf(fTmp, "%s/.config/sfm/sfm.conf", getenv("HOME"));
        i = 0;
    }
    fclose(f);
    ini = iniparser_load(fTmp);
    strcpy(tEditor, iniparser_getstring(ini, "global:text editor", "gedit"));
    strcpy(term, iniparser_getstring(ini, "global:terminal emulator", "xterm"));
    strcpy(loc, iniparser_getstring(ini, "global:locale", "en_US.UTF-8"));
    strcpy(sTmp, iniparser_getstring(ini, "binds:another window", "a"));
    if (sTmp[0]=='\0')
        keys[i++] = 9;
    else
        keys[i++] = sTmp[0];
    while (i<ACTIONS){
        sprintf(fTmp, "binds:%s", actions[i]);
        strcpy(sTmp, iniparser_getstring(ini, fTmp, NULL));
        keys[i++] = sTmp[0];
    }
    getUserBinds(ubinds, numbOfBinds, ini);
    getFileTypes(ftypes, numbOfFTypes, ini);
    iniparser_freedict(ini);
    endwin();
}
