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
#include "iniparser/src/iniparser.c"
#include "iniparser/src/dictionary.c"

typedef struct{
        WINDOW *win;
        int currentLine;
        int top;
    } wnd;

typedef struct{
        char path[4096];
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
    p->f.numbOfLines = scandir(p->f.path, &p->f.files, (p->f.hid)?0:slt, alphasort);
}

void setKeys(FILE *f, const char **cnf){
    char tmp[1024];
    int sch, ssch=0;

    echo();
    clear();
    printw("Enter name of your text editor: ");
    scanw("%[^\n]", tmp);
    fprintf(f, "[Global]\nText editor = %s ;\n", tmp);
    clear();
    printw("Enter name of your terminal emulator\nwith flags to execute: ");
    scanw("%[^\n]", tmp);
    fprintf(f, "Terminal emulator = %s ;\n[Binds]\n", tmp);
    noecho();
    while (ssch<15){
        clear();
        printw("Enter symbol to action \"%s\": ", cnf[ssch]);
        sch=getch();
        fprintf(f, "%s = %c ;\n", cnf[ssch++], sch);
    }
}

void readConf(part *p, part *pp, char *td, char *term, int keys[]){
    char tmp[128], ttmp[8];
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
    strcpy(p->f.path, iniparser_getstring(ini, "main:lpath", NULL));
    strcpy(pp->f.path, iniparser_getstring(ini, "main:rpath", NULL));
    p->f.hid = iniparser_getint(ini, "main:lhid", -1);
    pp->f.hid = iniparser_getint(ini, "main:rhid", -1);
    iniparser_freedict(ini);
    strcpy(tmp, getenv("HOME"));
    strcat(tmp, "/.config/sfm/sfm.conf");
    f=fopen(tmp, "r");
    if (!f){
        printw("Main .conf file not found. (C)onfigure now or use (d)efault?");
        sch=getch();
        f=fopen(tmp, "w");
        if (sch=='d'){
            fprintf(f,"[Global]\nText editor = gedit ;\nTerminal emulator = xterm ;\n");
            fprintf(f,"[Binds]\nAnother window = a ;\nQuit = q ;\nHidden mode = h ;\nSame folder = s ;\nChange folder = c ;\nNew folder = M ;\nCopy = C ;\nMove = m ;\nRemove = r ;\nRename = R ;\nParent folder = u '\nGo to line = g ;\nExecute = X ;\nInfo = i ;\nExecute command = k ;\n");
        }
        else
            setKeys(f, cnf);
        fclose(f);
        clear();
        printw("Config saved to ~/.config/sfm/sfm.conf\nPress enter");
        getch();
        strcpy(tmp, getenv("HOME"));
        strcat(tmp, "/.config/sfm/sfm.conf");
        sch=0;
    }
    ini = iniparser_load(tmp);
    strcpy(td, iniparser_getstring(ini, "global:text editor", NULL));
    strcpy(term, iniparser_getstring(ini, "global:terminal emulator", NULL));
    strcpy(ttmp, iniparser_getstring(ini, "binds:another window", NULL));
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
}

void saveConf(part *p, part *pp){
    char tmp[128];

    strcpy(tmp, getenv("HOME"));
    strcat(tmp, "/.config/sfm/tmp");
    FILE *f=fopen(tmp, "w");
    fprintf(f, "[Main]\nlPath = %s ;\n", p->f.path);
    fprintf(f, "rPath = %s ;\n", pp->f.path);
    fprintf(f, "lHid = %d ;\n", p->f.hid);
    fprintf(f, "rHid = %d ;\n", pp->f.hid);
    fclose(f);
}

void setTop(wnd *w, int mx, int dd){
    if (dd)
        w->currentLine=0;
    if (w->currentLine>mx-4)
        w->top=w->currentLine-(mx-5);
    else
        w->top=0;
}

void draw(part p, int mx){
    int tY=1;
    struct stat tset;
    char tmp[4096];

    wmove(p.w.win, 1, 1);
    while ((tY<mx-3)&&(tY-1<p.f.numbOfLines)){
        wprintw(p.w.win, "%s", p.f.files[p.w.top + tY-1]->d_name);
        strcpy(tmp, p.f.path);
        strcat(tmp, p.f.files[p.w.top + tY-1]->d_name);
        stat(tmp, &tset);
        if (S_ISDIR(tset.st_mode))
            wprintw(p.w.win, "/");
        if (((tset.st_mode|S_IXUSR)==tset.st_mode)&&(!S_ISDIR(tset.st_mode)))
            wprintw(p.w.win, "*");
        if (S_ISLNK(tset.st_mode))
            wprintw(p.w.win, "->");
        wmove(p.w.win, ++tY, 1);
    }
}

void kUp(wnd *w, int mx, int ml){
    --w->currentLine;
    if (w->currentLine<0){
        w->currentLine=mx-1;
        if (mx>ml-4)
            w->top=w->currentLine-(ml-5);
        else
            w->top=0;
    }
    if (w->currentLine<w->top)
        --w->top;
}

void kDown(wnd *w, int mx, int ml){
    ++w->currentLine;
    if (w->currentLine>mx-1){
        w->currentLine=0;
        w->top=0;
    }
    if (w->currentLine>w->top+(ml-5))
        ++w->top;
}

void kEnter(part *p, char *td){
    char tmp[4096], ttmp[4096];
    struct stat tset;

    if (p->f.numbOfLines){
        strcpy(tmp, p->f.path);
        strcat(tmp, p->f.files[p->w.currentLine]->d_name);
        stat(tmp, &tset);
        if (S_ISDIR(tset.st_mode)){
            strcat(p->f.path, p->f.files[p->w.currentLine]->d_name);
            strcat(p->f.path, "/");
        }
        if ((S_ISREG(tset.st_mode))&&((tset.st_mode|S_IXUSR)!=tset.st_mode)){
            strcpy(ttmp, td);
            strcat(ttmp, " ");
            strcat(ttmp, tmp);
            strcat(ttmp, " >/dev/null &");
            system(ttmp);
        }
    }
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
    scanw("%s", o);
    noecho();
}

void reSize(wnd *w, wnd *w1, int *mc, int *ml){
    int tmX, tmY;

    getmaxyx(stdscr, tmY, tmX);
    if ((*mc!=tmY)||(*ml!=tmX)){
        delwin(w->win);
        delwin(w1->win);
        wclear(stdscr);
        w->win = newwin(tmY-2, tmX/2, 1, 0);
        w1->win = newwin(tmY-2, tmX/2, 1, tmX/2);
        (*mc)=tmY;
        (*ml)=tmX;
    }
}

void goToLine(part *p, int mx){
    echo();
    printw("New position: ");
    scanw("%d", &p->w.currentLine);
    noecho();
    if ((--p->w.currentLine<0)||(p->w.currentLine>p->f.numbOfLines)){
        p->w.top=0;
        p->w.currentLine=0;
    }
    setTop(&p->w, mx, 0);
}

void chDir(fl *f){
    char tmp[1024];

    dAsk("Name of folder", tmp);
    if (tmp[strlen(tmp)-1]!='/')
        strcat(tmp, "/");
    if (tmp[0]=='/')
        strcpy(f->path, tmp);
    else
        strcat(f->path, tmp);
}

void newDir(fl *f){
    char tmp[1024], ttmp[1024];

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
}

void cpItem(part *p, part *pp){
    char tmp[1024];
    struct stat tset;

    strcpy(tmp, "cp ");
    strcat(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    stat(tmp, &tset);
    if (S_ISDIR(tset.st_mode))
        strcat(tmp, " -r");
    strcat(tmp, " ");
    strcat(tmp, pp->f.path);
    strcat(tmp, " &");
    system(tmp);
}

void moveItem(part *p, part *pp){
    char tmp[1024];

    strcpy(tmp, "mv ");
    strcat(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    strcat(tmp, " ");
    strcat(tmp, pp->f.path);
    strcat(tmp, " &");
    system(tmp);
}

void rmItem(part *p){
    char tmp[1024];

    dAsk("Are you SURE?", tmp);
    if (tmp[0]=='y'){
        strcpy(tmp, "rm -rf ");
        strcat(tmp, p->f.path);
        strcat(tmp, p->f.files[p->w.currentLine]->d_name);
        strcat(tmp, " &");
        system(tmp);
    }
}

void renItem(part *p){
    char tmp[1024], ttmp[1024];

    dAsk("New name", tmp);
    strcpy(ttmp, "mv ");
    strcat(ttmp, p->f.path);
    strcat(ttmp, p->f.files[p->w.currentLine]->d_name);
    strcat(ttmp, " ");
    strcat(ttmp, p->f.path);
    strcat(ttmp, tmp);
    system(ttmp);
}

void exec(part *p, char *term, int nw){
    char tmp[1024], ttmp[1024];

    struct stat tset;
    strcpy(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    stat(tmp, &tset);
    if ((tset.st_mode|S_IXUSR)==tset.st_mode){
        if (nw){
            strcpy(ttmp, term);
            strcat(ttmp, " ");
            strcat(ttmp, tmp);
        }
        else
            strcpy(ttmp, tmp);
        system(strcat(ttmp, " > /dev/null &"));
    }
}

void cmdKey(void){
    char tmp[1024];

    dAsk("Command", tmp);
    strcat(tmp, " &");
    system(tmp);
}

void gInfo(part *p, wnd *w){
    int tY=1;
    char tmp[1024];
    struct stat tset;
    short int octarray[9] = {0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001};
    const char rs[]="rwx";

    wclear(w->win);
    box(w->win, 0, 0);
    strcpy(tmp, p->f.path);
    strcat(tmp, p->f.files[p->w.currentLine]->d_name);
    stat(tmp, &tset);
    mvwprintw(w->win, tY++, 1, "Name: %s", p->f.files[p->w.currentLine]->d_name);
    mvwprintw(w->win, tY++, 1, "Size: %dB", tset.st_size);
    mvwprintw(w->win, tY++, 1, "Access mode: ");
    for (tY=0;tY<9;tY++){
        if (tset.st_mode & octarray[tY])
            wprintw(w->win, "%c", rs[tY%3]);
        else
            wprintw(w->win, "-");
        ((tY+1)%3==0) ? wprintw(w->win, " ") : 0;
    }
    move(23, 0);
    wrefresh(w->win);
    getch();
}

void kHome(wnd *w){
    w->currentLine=0;
    w->top=0;
}

void kEnd(part *p, int mx){
    p->w.currentLine = p->f.numbOfLines-1;
    setTop(&p->w, mx, 0);
}

void nfo(part *p, int mx){
    int sch;
    char *rd;

    mvwchgat(p->w.win, p->w.currentLine-p->w.top+1, 1, mx/2-2, A_REVERSE, 0, NULL);
    if (strlen(p->f.path)<(mx-15))
        mvprintw(0, 0, "%s", p->f.path);
    else{
        rd=&p->f.path[strlen(p->f.path)-mx+17];
        mvprintw(0, 0, "..");
        printw("%s", rd);
    }
    mvprintw(0, mx-15, "|%d/%d", p->w.currentLine+1, p->f.numbOfLines);
}

int main(int argc, char *argv[]){
    int mRow=0, mCol=0, lOrR=0, cmd, inCy=1, keys[15];         //lOrR = left ot right (selected)
    part lPart, rPart, *tmpCPt, *tmpAPt;
    char tmp[32], TEd[32], term[64];
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;

    initscr();
    noecho();
    keypad(stdscr, true);
    readConf(&lPart, &rPart, TEd, term, keys);
    if (argc == 2)
        strcpy(lPart.f.path, argv[1]);
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
        draw(lPart, mRow);
        draw(rPart, mRow);
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
        else if ((cmd==KEY_ENTER)||(cmd=='\n')){
            kEnter(tmpCPt, TEd);
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
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
            exec(tmpCPt, term, 1);
        else if (cmd==keys[13])
            gInfo(tmpCPt, &tmpAPt->w);
        else if (cmd==keys[14])
            cmdKey();
        else if (cmd==KEY_F(5)){
            fillList(&lPart);
            fillList(&rPart);
            setTop(&lPart.w, mRow, 1);
            setTop(&rPart.w, mRow, 1);
            }
        else if (cmd==KEY_F(6)){
            saveConf(&lPart, &rPart);
            readConf(&lPart, &rPart, TEd, term, keys);
            }
        else if (cmd==KEY_HOME)
            kHome(&tmpCPt->w);
        else if (cmd=KEY_END)
            kEnd(tmpCPt, mRow);
    }

    endwin();
    saveConf(&lPart, &rPart);
    inCy=0;
    while (lPart.f.files[inCy])
        free(lPart.f.files[inCy++]);
    free(lPart.f.files);
    inCy=0;
    while (rPart.f.files[inCy])
        free(rPart.f.files[inCy++]);
    free(rPart.f.files);
    return 0;
}
