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

typedef struct{
        WINDOW *win;
        int currentLine;
        int top;
    } wnd;

typedef struct{
        char *path;
        struct dirent **files;
        int numbOfLines;
        int hid;
    } fl;

typedef struct{
        wnd w;
        fl f;
    } part;

int slt(const struct dirent *d){
     return strncmp(d->d_name, ".", 1);
}

void fillList(part *p){
    int sch=0;

    while (sch<p->f.numbOfLines)
        free(p->f.files[sch++]);
    p->f.numbOfLines = scandir(p->f.path, &p->f.files, (p->f.hid)?0:slt, alphasort);
}

void execInBkg(char *tmp){
    strcat(tmp, " > /dev/null 2> /dev/null &");
    system(tmp);
}

void setKeys(FILE *f, const char **cnf){
    char *tmp = (char *) malloc(sizeof(char)*4096);
    int sch, ssch=0;

    echo();
    clear();
    printw("Enter name of your text editor: ");
    scanw("%[^\n]", tmp);
    fprintf(f, "[Global]\nText editor = %s ;\n", tmp);
    clear();
    printw("Enter name of your terminal emulator\nwith flags to execute: ");
    scanw("%[^\n]", tmp);
    fprintf(f, "Terminal emulator = %s ;\n", tmp);
    fprintf(f, "Locale = %s ;\n[Binds]\n", getenv("LANG"));
    noecho();
    while (ssch<15){
        clear();
        printw("Enter symbol to action \"%s\": ", cnf[ssch]);
        sch=getch();
        fprintf(f, "%s = %c ;\n", cnf[ssch++], sch);
    }
    free(tmp);
}

void readConf(part *p, part *pp, char *td, char *term, int keys[], char *lc){
    char *tmp = (char *) malloc(sizeof(char)*128), *ttmp = (char *) malloc(sizeof(char)*4);
    const char *cnf[] = {"Another window", "Quit", "Hidden mode", "Same folder", "Change folder", "New folder", "Copy", "Move", "Remove", "Rename", "Parent folder", "Go to line", "Execute", "Info", "Execute command"};
    int sch=0, ssch=0;
    dictionary *ini;

    strcpy(tmp, getenv("HOME"));
    strcat(tmp, "/.config/sfm/tmp");
    FILE *f=fopen(tmp, "r");
    if (!f){
        system("mkdir -p ~/.config/sfm/");
        f=fopen(tmp, "w");
        fprintf(f, "[Main]\nlPath = / ;\nrPath = / ;\nlHid = 0 ;\nrHid = 0 ;\n");
    }
    fclose(f);
    ini = iniparser_load(tmp);
    strcpy(p->f.path, iniparser_getstring(ini, "main:lpath", "/"));
    strcpy(pp->f.path, iniparser_getstring(ini, "main:rpath", "/"));
    p->f.hid = iniparser_getint(ini, "main:lhid", 0);
    pp->f.hid = iniparser_getint(ini, "main:rhid", 0);
    iniparser_freedict(ini);
    strcpy(tmp, getenv("HOME"));
    strcat(tmp, "/.config/sfm/sfm.conf");
    f=fopen(tmp, "r");
    if (!f){
        printw("Main .conf file not found. (C)onfigure now or use (d)efault?");
        sch=getch();
        f=fopen(tmp, "w");
        if (sch=='d'){
            fprintf(f,"[Global]\nText editor = gedit ;\nTerminal emulator = xterm ;\nLocale = en_US.UTF-8 ;\n");
            fprintf(f,"[Binds]\nAnother window = a ;\nQuit = q ;\nHidden mode = h ;\nSame folder = s ;\nChange folder = c ;\nNew folder = M ;\nCopy = C ;\nMove = m ;\nRemove = r ;\nRename = R ;\nParent folder = u ;\nGo to line = g ;\nExecute = X ;\nInfo = i ;\nExecute command = k ;\n");
        }
        else
            setKeys(f, cnf);
        fclose(f);
        clear();
        printw("Config saved to ~/.config/sfm/sfm.conf\nPress enter");
        getch();
        sch=0;
    }
    ini = iniparser_load(tmp);
    strcpy(td, iniparser_getstring(ini, "global:text editor", "gedit"));
    strcpy(term, iniparser_getstring(ini, "global:terminal emulator", "xterm"));
    strcpy(lc, iniparser_getstring(ini, "global:locale", "en_US.UTF-8"));
    strcpy(ttmp, iniparser_getstring(ini, "binds:another window", "a"));
    if (ttmp[0]=='\0')
        keys[sch++]=9;
    else
        keys[sch++]=ttmp[0];
    while (sch<15){
        strcpy(tmp, "binds:");
        strcat(tmp, cnf[sch]);
        strcpy(ttmp, iniparser_getstring(ini, tmp, NULL));
        keys[sch++]=ttmp[0];
    }
    iniparser_freedict(ini);
    free(tmp);
    free(ttmp);
}

void saveConf(part *p, part *pp){
    char *tmp = (char *) malloc(sizeof(char)*128);

    strcpy(tmp, getenv("HOME"));
    strcat(tmp, "/.config/sfm/tmp");
    FILE *f=fopen(tmp, "w");
    fprintf(f, "[Main]\nlPath = %s ;\n", p->f.path);
    fprintf(f, "rPath = %s ;\n", pp->f.path);
    fprintf(f, "lHid = %d ;\n", p->f.hid);
    fprintf(f, "rHid = %d ;\n", pp->f.hid);
    fclose(f);
    free(tmp);
}

void setTop(wnd *w, int mRow, int dd){
    if (dd)
        w->currentLine=0;
    if (w->currentLine>mRow-4)
        w->top=w->currentLine-(mRow-5);
    else
        w->top=0;
}

void draw(part p, int mRow, int mCol){
    int tY=1;
    struct stat tset;
    char *tmp = (char *) malloc(sizeof(char)*4096), *fStr = (char *) malloc(sizeof(char)*24);

    sprintf(fStr, "%%.%ds", mCol/2-3);
    wmove(p.w.win, 1, 1);
    while ((tY<mRow-3)&&(tY-1<p.f.numbOfLines)){
        wprintw(p.w.win, fStr, p.f.files[p.w.top + tY-1]->d_name);
        strcpy(tmp, p.f.path);
        strcat(tmp, p.f.files[p.w.top + tY-1]->d_name);
        stat(tmp, &tset);
        if (S_ISDIR(tset.st_mode))
            wprintw(p.w.win, "/");
        if ((tset.st_mode & S_IXUSR)&&(!S_ISDIR(tset.st_mode)))
            wprintw(p.w.win, "*");
        if (S_ISLNK(tset.st_mode))      //FIXME: try with S_IFLNK
            wprintw(p.w.win, ">");
        wmove(p.w.win, ++tY, 1);
    }
    free(tmp);
    free(fStr);
}

void kUp(wnd *w, int nmbOfLines, int mRow){
    --w->currentLine;
    if (w->currentLine<0){
        w->currentLine=nmbOfLines-1;
        if (nmbOfLines>mRow-4)
            w->top=w->currentLine-(mRow-5);
        else
            w->top=0;
    }
    if (w->currentLine<w->top)
        --w->top;
}

void kDown(wnd *w, int nmbOfLines, int mRow){
    ++w->currentLine;
    if (w->currentLine>nmbOfLines-1){
        w->currentLine=0;
        w->top=0;
    }
    if (w->currentLine>w->top+(mRow-5))
        ++w->top;
}

void kEnter(part *p, char *td, char *term, int mRow){
    char *tmp = (char *) malloc(sizeof(char)*4096), *ttmp = (char *) malloc(sizeof(char)*4096);
    struct stat tset;

    if (p->f.numbOfLines){
        strcpy(tmp, p->f.path);
        strcat(tmp, p->f.files[p->w.currentLine]->d_name);
        stat(tmp, &tset);
        if (S_ISDIR(tset.st_mode)){
            strcat(p->f.path, p->f.files[p->w.currentLine]->d_name);
            strcat(p->f.path, "/");
            fillList(p);
            setTop(&p->w, mRow, 1);
        }
        else if ((S_ISREG(tset.st_mode))&&(!(tset.st_mode & S_IXUSR))){
            strcpy(ttmp, td);
            strcat(ttmp, " ");
            strcat(ttmp, tmp);
            execInBkg(ttmp);
        }
        else if (tset.st_mode & S_IXUSR){
            strcpy(ttmp, term);
            strcat(ttmp, " ");
            strcat(ttmp, tmp);
            execInBkg(ttmp);
        }
    }
    free(tmp);
    free(ttmp);
}

void kPFld(fl *f){
    int i;

    for(i=strlen(f->path)-2;i>=0;i--)
        if (f->path[i]=='/'){
            f->path[i+1]='\0';
            break;
        }
}

void dAsk(char *i, char *o){
    echo();
    printw("%s: ", i);
    scanw("%[^\n]", o);
    noecho();
}

void reSize(wnd *w, wnd *w1, int *mRow, int *mCol){
    int tmX, tmY;

    getmaxyx(stdscr, tmY, tmX);
    if ((*mRow!=tmY)||(*mCol!=tmX)){
        delwin(w->win);
        delwin(w1->win);
        wclear(stdscr);
        w->win = newwin(tmY-2, tmX/2, 1, 0);
        w1->win = newwin(tmY-2, tmX/2, 1, tmX/2);
        *mRow=tmY;
        *mCol=tmX;
    }
}

void goToLine(part *p, int mRow){
    echo();
    printw("New position: ");
    scanw("%d", &p->w.currentLine);
    noecho();
    if ((--p->w.currentLine<0)||(p->w.currentLine>p->f.numbOfLines)){
        p->w.top=0;
        p->w.currentLine=0;
    }
    setTop(&p->w, mRow, 0);
}

void chDir(fl *f){
    char *tmp = (char *) malloc(sizeof(char)*4096);

    dAsk("Name of folder", tmp);
    if (tmp[strlen(tmp)-1]!='/')
        strcat(tmp, "/");
    if (tmp[0]=='/')
        strcpy(f->path, tmp);
    else
        strcat(f->path, tmp);
    free(tmp);
}

void newDir(fl *f){
    char *tmp = (char *) malloc(sizeof(char)*4096), *ttmp = (char *) malloc(sizeof(char)*4096);

    dAsk("Name of new folder", tmp);
    if (tmp[0]=='/'){
        strcpy(ttmp, "mkdir ");
        system(strcat(ttmp, tmp));
    }
    else{
        strcpy(ttmp, "mkdir ");
        strcat(ttmp, f->path);
        strcat(ttmp, tmp);
        system(ttmp);
    }
    free(tmp);
    free(ttmp);
}

void cpItem(part *p, part *pp){
    char *tmp = (char *) malloc(sizeof(char)*4096);
    struct stat tset;

    strcpy(tmp, "cp \"");
    strcat(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    stat(tmp, &tset);
    strcat(tmp, "\"");
    if (S_ISDIR(tset.st_mode))
        strcat(tmp, " -r");
    strcat(tmp, " \"");
    strcat(tmp, pp->f.path);
    strcat(tmp, "\" &");
    system(tmp);
    free(tmp);
}

void moveItem(part *p, part *pp){
    char *tmp = (char *) malloc(sizeof(char)*4096);

    strcpy(tmp, "mv \"");
    strcat(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    strcat(tmp, "\" \"");
    strcat(tmp, pp->f.path);
    strcat(tmp, "\" &");
    system(tmp);
    free(tmp);
}

void rmItem(part *p){
    char *tmp = (char *) malloc(sizeof(char)*4096);

    dAsk("Are you SURE?", tmp);
    if (tmp[0]=='y'){
        strcpy(tmp, "rm -rf \"");
        strcat(tmp, p->f.path);
        strcat(tmp, p->f.files[p->w.currentLine]->d_name);
        strcat(tmp, "\" &");
        system(tmp);
    }
    free(tmp);
}

void renItem(part *p){
    char *tmp = (char *) malloc(sizeof(char)*4096), *ttmp = (char *) malloc(sizeof(char)*4096);

    dAsk("New name", tmp);
    strcpy(ttmp, "mv \"");
    strcat(ttmp, p->f.path);
    strcat(ttmp, p->f.files[p->w.currentLine]->d_name);
    strcat(ttmp, "\" \"");
    strcat(ttmp, p->f.path);
    strcat(ttmp, tmp);
    strcat(ttmp, "\"");
    system(ttmp);
    free(tmp);
    free(ttmp);
}

void cmdKey(part *p){
    char *tmp = (char *) malloc(sizeof(char)*4096);

    dAsk("Command", tmp);
    if (p){
        strcat(tmp, "\"");
        strcat(tmp, p->f.path);
        strcat(tmp, p->f.files[p->w.currentLine]->d_name);
        strcat(tmp, "\"");
    }
    FILE *f=fopen("log", "w");
    fprintf(f, "=%s=", tmp);
    fclose(f);
    execInBkg(tmp);
    free(tmp);
}

void gInfo(part *p, wnd *w, int mCol){
    int tY=1, sch;
    char *tmp = (char *) malloc(sizeof(char)*4096);
    struct stat tset;
    short int octarray[9] = {0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001};
    const char rs[]="rwx";

    wclear(w->win);
    box(w->win, 0, 0);
    strcpy(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    stat(tmp, &tset);
    mvwprintw(w->win, tY, 1, "Name: %s", p->f.files[p->w.currentLine]->d_name);
    tY += strlen(p->f.files[p->w.currentLine]->d_name)/(mCol/2) + 1;
    mvwprintw(w->win, tY++, 1, "Size: %dB", tset.st_size);
    mvwprintw(w->win, tY++, 1, "Access mode: ");
    for (sch=0;sch<9;sch++){
        if (tset.st_mode & octarray[sch])
            wprintw(w->win, "%c", rs[sch%3]);
        else
            wprintw(w->win, "-");
        ((sch+1)%3==0) ? wprintw(w->win, " ") : 0;
    }
    wprintw(w->win, "(%o)", (tset.st_mode & S_IRWXU)+(tset.st_mode & S_IRWXG)+(tset.st_mode & S_IRWXO));
    mvwprintw(w->win, tY++, 1, "Owner: ");
    sprintf(tmp, "getent passwd %d > /tmp/sfm-Owner", tset.st_uid);
    system(tmp);
    FILE *f=fopen("/tmp/sfm-Owner", "r");
    fscanf(f, "%[^:]", tmp);
    fclose(f);
    wprintw(w->win, "%s", tmp);
    move(23, 0);
    wrefresh(w->win);
    getch();
    free(tmp);
}

void kHome(wnd *w){
    w->currentLine=0;
    w->top=0;
}

void kEnd(part *p, int mRow){
    p->w.currentLine = p->f.numbOfLines-1;
    setTop(&p->w, mRow, 0);
}

void nfo(part *p, int mCol){
    int sch;
    char *rd;

    mvwchgat(p->w.win, p->w.currentLine-p->w.top+1, 1, mCol/2-2, A_REVERSE, 0, NULL);
    if (strlen(p->f.path)<(mCol-15))
        mvprintw(0, 0, "%s", p->f.path);
    else{
        rd=&p->f.path[strlen(p->f.path)-mCol+17];
        mvprintw(0, 0, "..");
        printw("%s", rd);
    }
    mvprintw(0, mCol-15, "|%d/%d", p->w.currentLine+1, p->f.numbOfLines);
}

int main(int argc, char **argv){
    int mRow=0, mCol=0, lOrR=0, cmd, inCy=1, keys[15];         //lOrR = left ot right (selected)
    part lPart, rPart, *tmpCPt, *tmpAPt;
    char tmp[32], TEd[32], term[64], lc[32];
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;
    lPart.f.path = (char *) malloc(sizeof(char)*4096);
    rPart.f.path = (char *) malloc(sizeof(char)*4096);

    readConf(&lPart, &rPart, TEd, term, keys, lc);
    if (argc == 2)
        strcpy(lPart.f.path, argv[1]);
    if (!setlocale(LC_ALL, lc)){
      printf("Can't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf");
      getch();
    }
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
        draw(lPart, mRow, mCol);
        draw(rPart, mRow, mCol);
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
            kEnter(tmpCPt, TEd, term, mRow);
        else if (cmd==keys[0])
            lOrR = !lOrR;
        else if (cmd==keys[1]){
            dAsk("Are you sure, quit?", tmp);
            if (tmp[0]=='y')
                inCy=0;
            }
        else if (cmd==keys[2]){
            tmpCPt->f.hid = !tmpCPt->f.hid;
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
            newDir(&tmpCPt->f);
            fillList(tmpCPt);
            }
        else if (cmd==keys[6])
            cpItem(tmpCPt, tmpAPt);
        else if (cmd==keys[7])
            moveItem(tmpCPt, tmpAPt);
        else if (cmd==keys[8])
            rmItem(tmpCPt);
        else if (cmd==keys[9]){
            renItem(tmpCPt);
            fillList(tmpCPt);
            }
        else if (cmd==keys[10]){
            kPFld(&tmpCPt->f);
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
        else if (cmd==keys[11])
            goToLine(tmpCPt, mRow);
        else if (cmd==keys[12])
            cmdKey(tmpCPt);
        else if (cmd==keys[13])
            gInfo(tmpCPt, &tmpAPt->w, mCol);
        else if (cmd==keys[14])
            cmdKey(NULL);
        else if (cmd==KEY_F(5)){
            fillList(&lPart);
            fillList(&rPart);
            setTop(&lPart.w, mRow, 1);
            setTop(&rPart.w, mRow, 1);
            }
        else if (cmd==KEY_F(6)){
            saveConf(&lPart, &rPart);
            readConf(&lPart, &rPart, TEd, term, keys, lc);
            }
        else if (cmd==KEY_HOME)
            kHome(&tmpCPt->w);
        else if (cmd=KEY_END)
            kEnd(tmpCPt, mRow);
    }

    endwin();
    saveConf(&lPart, &rPart);
    inCy=0;
    while (inCy<lPart.f.numbOfLines)
        free(lPart.f.files[inCy++]);
    free(lPart.f.files);
    inCy=0;
    while (inCy<rPart.f.numbOfLines)
        free(rPart.f.files[inCy++]);
    free(rPart.f.files);
    free(lPart.f.path);
    free(rPart.f.path);
    return 0;
}
