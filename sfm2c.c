/*
 * sfm2b.c
 *
 * Copyright 2011 illumin <illumin.95@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * v=0.02b
 * d=2011-06-19
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include "iniparser/src/iniparser.c"
#include "iniparser/src/dictionary.c"

#define ACTIONS 9

typedef struct{
        WINDOW *win;
        int currentLine;
        int top;
    } wnd;

typedef struct{
        char *path;
        struct dirent **files;
        int numbOfLines;
        int showHidden;
    } fl;

typedef struct{
        wnd w;
        fl f;
    } part;

typedef struct{
        char key;
        char *cmd;
    } bnd;

char *fTmp, *sTmp;

int slt(const struct dirent *d){
     return strncmp(d->d_name, ".", 1);
}

void ask(char *i, char *o){
    echo();
    printw("%s: ", i);
    scanw("%[^\n]", o);
    noecho();
}

void mFree(void **ptr, int len){
    while(len--)
        free(ptr[len]);
    free(ptr);
}

void fillList(part *fPart){
    fPart->f.numbOfLines ? mFree((void **)fPart->f.files, fPart->f.numbOfLines) : 0;
    fPart->f.numbOfLines = scandir(fPart->f.path, &fPart->f.files, (fPart->f.showHidden)?0:slt, alphasort);
}

void parseCmd(part *fPart, part *sPart, char *cmd){
    char *outCmd = (char *) malloc(sizeof(char)*1024),
         *tmp;

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
        else
            strcat(outCmd, tmp);
        tmp = strtok(NULL, "%");    
    }
    system(outCmd);
    free(tmp);
    free(outCmd);
}

void execInBkg(char *cmd){
    strcat(cmd, " > /dev/null 2> /dev/null &");
    system(cmd);
}


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
}

void readConf(part *fPart, part *sPart, char *tEditor, char *term, int *keys, char *loc){
    const char *actions[ACTIONS] = {"Another window", "Quit", "Hidden mode", "Same folder", "Change folder", "Parent folder", "Go to line", "Info", "Execute command"};
    int i=0;
    dictionary *ini;

    initscr();
    strcpy(fTmp, getenv("HOME"));
    strcat(fTmp, "/.config/sfm/tmp");
    FILE *f = fopen(fTmp, "r");
    if (!f){
        system("mkdir -p ~/.config/sfm/");
        f=fopen(fTmp, "w");
        fprintf(f, "[Main]\nlPath = / ;\nrPath = / ;\nlHid = 0 ;\nrHid = 0 ;\n");
    }
    fclose(f);
    ini = iniparser_load(fTmp);
    strcpy(fPart->f.path, iniparser_getstring(ini, "main:lpath", "/"));
    strcpy(sPart->f.path, iniparser_getstring(ini, "main:rpath", "/"));
    fPart->f.showHidden = iniparser_getint(ini, "main:lhid", 0);
    sPart->f.showHidden = iniparser_getint(ini, "main:rhid", 0);
    iniparser_freedict(ini);
    strcpy(fTmp, getenv("HOME"));
    strcat(fTmp, "/.config/sfm/sfm.conf");
    f = fopen(fTmp, "r");
    if (!f){
        printw("Main .conf file not found. (C)onfigure now or use (d)efault?");
        i = getch();
        f = fopen(fTmp, "w");
        if (i=='d')
            fprintf(f,"[Global]\nText editor = gedit ;\nTerminal emulator = xterm ;\nLocale = en_US.UTF-8 ;\n"
                      "[Binds]\nAnother window = a ;\nQuit = q ;\nHidden mode = h ;\nSame folder = s ;\nChange folder = c ;\n"
                      "Parent folder = u ;\nGo to line = g ;\nInfo = i ;\nExecute command = k ;\n"
                      "[uBinds]\n");
        else
            setKeys(f, actions);
        fclose(f);
        clear();
        printw("Config saved to ~/.config/sfm/sfm.conf\nYou can set user-binds in.\nPress enter");
        getch();
        i = 0;
    }
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
    iniparser_freedict(ini);
    endwin();
}

void getUserBinds(int *numbOfBinds, bnd ***ubinds){
    const char *symbols = "qwertyuiopasdfghjklzxcvbnm";
    *ubinds = (bnd **) malloc(sizeof(bnd *));
    int i=0;
    dictionary *ini;

    strcpy(fTmp, getenv("HOME"));
    strcat(fTmp, "/.config/sfm/sfm.conf");
    ini = iniparser_load(fTmp);
    while (i<strlen(symbols)){
        sprintf(fTmp, "ubinds:%c", symbols[i]);
        strcpy(fTmp, iniparser_getstring(ini, fTmp, "\n"));
        if (fTmp[0]!='\n'){
            ((*numbOfBinds)>0) ? *ubinds = (bnd **) realloc(*ubinds, sizeof(bnd *)*((*numbOfBinds)+1)) : 0;
            (*ubinds)[*numbOfBinds] = (bnd *) malloc(sizeof(bnd));
            (*ubinds)[*numbOfBinds]->key = symbols[i];
            (*ubinds)[*numbOfBinds]->cmd = (char *) malloc((size_t)strlen(fTmp)+1);
            strcpy((*ubinds)[*numbOfBinds]->cmd, fTmp);
            ++(*numbOfBinds);
        }
        ++i;
    }
    iniparser_freedict(ini);
}

void saveConf(part *fPart, part *sPart){
    strcpy(fTmp, getenv("HOME"));
    strcat(fTmp, "/.config/sfm/tmp");
    FILE *f = fopen(fTmp, "w");
    fprintf(f, "[Main]\nlPath = %s ;\nrPath = %s ;\nlHid = %d ;\nrHid = %d ;\n",
               fPart->f.path, sPart->f.path, fPart->f.showHidden, sPart->f.showHidden);
    fclose(f);
}

void setTop(wnd *fWind, int mRow, int mkSt){
    if (mkSt)
        fWind->currentLine=0;
    if (fWind->currentLine>mRow-4)
        fWind->top = fWind->currentLine-(mRow-5);
    else
        fWind->top = 0;
}

void draw(part *fPart, int mRow, int mCol){
    int tY=1;
    struct stat fStat;

    wmove(fPart->w.win, 1, 1);
    while ((tY<mRow-3)&&(tY-1<fPart->f.numbOfLines)){
        wprintw(fPart->w.win, "%.*s", mCol/2-3, fPart->f.files[fPart->w.top + tY-1]->d_name);
        strcpy(fTmp, fPart->f.path);
        strcat(fTmp, fPart->f.files[fPart->w.top + tY-1]->d_name);
        stat(fTmp, &fStat);
        if (S_ISDIR(fStat.st_mode))
            wprintw(fPart->w.win, "/");
        if ((fStat.st_mode & S_IXUSR)&&(!S_ISDIR(fStat.st_mode)))
            wprintw(fPart->w.win, "*");
        if (S_ISLNK(fStat.st_mode))      //FIXME: try with S_IFLNK
            wprintw(fPart->w.win, ">");
        wmove(fPart->w.win, ++tY, 1);
    }
}

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

void kDown(wnd *fWind, int numbOfLines, int mRow){
    ++fWind->currentLine;
    if (fWind->currentLine>numbOfLines-1){
        fWind->currentLine = 0;
        fWind->top = 0;
    }
    if (fWind->currentLine>fWind->top+(mRow-5))
        ++fWind->top;
}

void kEnter(part *fPart, char *tEditor, char *term, int mRow){
    struct stat fStat;

    if (fPart->f.numbOfLines){
        strcpy(fTmp, fPart->f.path);
        strcat(fTmp, fPart->f.files[fPart->w.currentLine]->d_name);
        stat(fTmp, &fStat);
        if (S_ISDIR(fStat.st_mode)){
            strcat(fPart->f.path, fPart->f.files[fPart->w.currentLine]->d_name);
            strcat(fPart->f.path, "/");
            fillList(fPart);
            setTop(&fPart->w, mRow, 1);
        }
        else if ((S_ISREG(fStat.st_mode))&&(!(fStat.st_mode & S_IXUSR))){
            strcpy(sTmp, tEditor);
            strcat(sTmp, " ");
            strcat(sTmp, fTmp);
            execInBkg(sTmp);
        }
        else if (fStat.st_mode & S_IXUSR){
            strcpy(sTmp, term);
            strcat(sTmp, " ");
            strcat(sTmp, fTmp);
            execInBkg(sTmp);
        }
    }
}

void kPFld(fl *fFold){
    int i;

    for(i=strlen(fFold->path)-2;i>=0;i--)
        if (fFold->path[i]=='/'){
            fFold->path[i+1]='\0';
            break;
        }
}

void reSize(wnd *fWind, wnd *sWind, int *mRow, int *mCol){
    int tmX, tmY;

    getmaxyx(stdscr, tmY, tmX);
    if ((*mRow!=tmY)||(*mCol!=tmX)){
        delwin(fWind->win);
        delwin(sWind->win);
        wclear(stdscr);
        fWind->win = newwin(tmY-2, tmX/2, 1, 0);
        sWind->win = newwin(tmY-2, tmX/2, 1, tmX/2);
        *mRow=tmY;
        *mCol=tmX;
    }
}

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

void chDir(fl *fFold){
    ask("Name of folder", fTmp);
    if (!strcmp(fTmp, "")){
        if (fTmp[strlen(fTmp)-1]!='/')
            strcat(fTmp, "/");
        if (fTmp[0]=='/')
            strcpy(fFold->path, fTmp);
        else
            strcat(fFold->path, fTmp);
    }
}

void cmdKey(){
    ask("Command", fTmp);
    if (strcmp(fTmp, ""))
        system(fTmp);
}

char *pSize(long int s){
    const char *sizes[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB"};
    int i=0;
    double sz = (double) s;

    while (sz>1024){
        sz /= 1024;
        ++i;
    }
    sprintf(fTmp, "%.2lf %s", sz, sizes[i]);
    return fTmp;
}

void gInfo(part *fPart, wnd *fWind, int mCol){
    const short int octarray[9] = {0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001};
    const char *rwx="rwx";
    int tY=1, i;
    struct stat fStat;

    wclear(fWind->win);
    box(fWind->win, 0, 0);
    strcpy(fTmp, fPart->f.path);
    strcat(fTmp, fPart->f.files[fPart->w.currentLine]->d_name);
    stat(fTmp, &fStat);
    mvwprintw(fWind->win, tY, 1, "Name: %s", fPart->f.files[fPart->w.currentLine]->d_name);
    tY += (strlen(fPart->f.files[fPart->w.currentLine]->d_name)+6)/(mCol/2) + 1;
    mvwprintw(fWind->win, tY++, 1, "Size: %s", pSize(fStat.st_size));
    mvwprintw(fWind->win, tY++, 1, "Access mode: ");
    for (i=0;i<9;i++){
        if (fStat.st_mode & octarray[i])
            wprintw(fWind->win, "%c", rwx[i%3]);
        else
            wprintw(fWind->win, "-");
        ((i+1)%3==0) ? wprintw(fWind->win, " ") : 0;
    }
    wprintw(fWind->win, "(%o)", (fStat.st_mode & S_IRWXU)+(fStat.st_mode & S_IRWXG)+(fStat.st_mode & S_IRWXO));
    mvwprintw(fWind->win, tY++, 1, "Owner: ");
    sprintf(fTmp, "getent passwd %d > /tmp/sfm-Owner", fStat.st_uid);
    system(fTmp);
    FILE *f = fopen("/tmp/sfm-Owner", "r");
    fscanf(f, "%[^:]", fTmp);
    fclose(f);
    wprintw(fWind->win, "%s", fTmp);
    move(23, 0);
    wrefresh(fWind->win);
    getch();
}

void kHome(wnd *fWind){
    fWind->currentLine = 0;
    fWind->top = 0;
}

void kEnd(part *fPart, int mRow){
    fPart->w.currentLine = fPart->f.numbOfLines-1;
    setTop(&fPart->w, mRow, 0);
}

void nfo(part *fPart, int mCol){
    char *rd;

    mvwchgat(fPart->w.win, fPart->w.currentLine-fPart->w.top+1, 1, mCol/2-2, A_REVERSE, 0, NULL);
    if (strlen(fPart->f.path)<(mCol-15))
        mvprintw(0, 0, "%s", fPart->f.path);
    else{
        rd=&fPart->f.path[strlen(fPart->f.path)-mCol+17];
        mvprintw(0, 0, "..");
        printw("%s", rd);
    }
    mvprintw(0, mCol-15, "|%d/%d", fPart->w.currentLine+1, fPart->f.numbOfLines);
}

int main(int argc, char **argv){
    int mRow=0, mCol=0, lOrR=0, cmd, inCy=1, keys[15], numbOfBinds=0;         //lOrR = left ot right (selected) //cob = count of binds
    part lPart, rPart, *tmpCPt, *tmpAPt;
    char tEditor[32], term[64], loc[32];
    bnd **ubinds=NULL;
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;
    lPart.f.path = (char *) malloc(sizeof(char)*4096);
    rPart.f.path = (char *) malloc(sizeof(char)*4096);
    fTmp = (char *) malloc(sizeof(char)*8192);
    sTmp = (char *) malloc(sizeof(char)*8192);

    readConf(&lPart, &rPart, tEditor, term, keys, loc);
    getUserBinds(&numbOfBinds, &ubinds);
    if (argc == 2)
        strcpy(lPart.f.path, argv[1]);
    if (!setlocale(LC_ALL, loc))
      fprintf(stderr, "Can't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf");
    initscr();
    noecho();
    keypad(stdscr, true);
    fillList(&lPart);
    fillList(&rPart);
    while (inCy){
        reSize(&lPart.w, &rPart.w, &mRow, &mCol);
        wclear(lPart.w.win);
        wclear(rPart.w.win);
        box(lPart.w.win, 0, 0);
        box(rPart.w.win, 0, 0);
        mvchgat(0, 0, -1, A_INVIS, 0, NULL);
        mvchgat(mRow-1, 0, -1, A_INVIS, 0, NULL);
        if ((cmd>='A')&&(cmd<='z'))
            mvprintw(0, mCol-1, "%c", cmd);
        tmpCPt = lOrR ? &rPart : &lPart;
        tmpAPt = lOrR ? &lPart : &rPart;
        draw(&lPart, mRow, mCol);
        draw(&rPart, mRow, mCol);
        nfo(tmpCPt, mCol);
        refresh();
        wrefresh(lPart.w.win);
        wrefresh(rPart.w.win);
        move(mRow-1, 0);
        cmd = getch();
        if (cmd==KEY_UP)
            kUp(&tmpCPt->w, tmpCPt->f.numbOfLines, mRow);
        else if (cmd==KEY_DOWN)
            kDown(&tmpCPt->w, tmpCPt->f.numbOfLines, mRow);
        else if ((cmd==KEY_ENTER)||(cmd=='\n'))
            kEnter(tmpCPt, tEditor, term, mRow);
        else if (cmd==keys[0])
            lOrR = !lOrR;
        else if (cmd==keys[1]){
            ask("Are you sure, quit?", fTmp);
            if (fTmp[0]=='y')
                inCy=0;
            }
        else if (cmd==keys[2]){
            tmpCPt->f.showHidden = !tmpCPt->f.showHidden;
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
        else if (cmd==keys[3]){
            strcpy(tmpAPt->f.path, tmpCPt->f.path);
            fillList(tmpAPt);
            setTop(&tmpAPt->w, mRow, 1);
            }
        else if (cmd==keys[4]){
            chDir(&tmpCPt->f);
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
        else if (cmd==keys[5]){
            kPFld(&tmpCPt->f);
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
        else if (cmd==keys[6])
            goToLine(tmpCPt, mRow);
        else if (cmd==keys[7])
            gInfo(tmpCPt, &tmpAPt->w, mCol);
        else if (cmd==keys[8])
            cmdKey();
        else if (cmd==KEY_F(5)){
            fillList(&lPart);
            fillList(&rPart);
            setTop(&lPart.w, mRow, 1);
            setTop(&rPart.w, mRow, 1);
            }
        else if (cmd==KEY_F(6)){
            saveConf(&lPart, &rPart);
            endwin();
            mFree((void **)lPart.f.files, lPart.f.numbOfLines);
            mFree((void **)rPart.f.files, rPart.f.numbOfLines);
            mFree((void **)ubinds, numbOfBinds);
            lPart.w.currentLine = 0; rPart.w.currentLine = 0;
            lPart.w.top = 0; rPart.w.top = 0;
            lPart.f.numbOfLines = 0; rPart.f.numbOfLines = 0;
            numbOfBinds = 0;
            readConf(&lPart, &rPart, tEditor, term, keys, loc);
            getUserBinds(&numbOfBinds, &ubinds);
            if (!setlocale(LC_ALL, loc))
              fprintf(stderr, "Can't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf");
            initscr();
            noecho();
            keypad(stdscr, true);
            fillList(&lPart);
            fillList(&rPart);
            inCy=1;
            }
        else if (cmd==KEY_HOME)
            kHome(&tmpCPt->w);
        else if (cmd==KEY_END)
            kEnd(tmpCPt, mRow);
        else {
            for(inCy=0;inCy<numbOfBinds;inCy++)
                if (cmd==ubinds[inCy]->key){
                    parseCmd(tmpCPt, tmpAPt, ubinds[inCy]->cmd);
                    break;
                }
            inCy = 1;
        }
    }

    endwin();
    saveConf(&lPart, &rPart);
    mFree((void **)lPart.f.files, lPart.f.numbOfLines);
    mFree((void **)rPart.f.files, rPart.f.numbOfLines);
    mFree((void **)ubinds, numbOfBinds);
    free(lPart.f.path);
    free(rPart.f.path);
    free(fTmp);
    free(sTmp);
    return 0;
}
