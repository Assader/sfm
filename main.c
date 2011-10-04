/*
 * main.c
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
 * v=0.03a
 * d=2011-09-29
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include <ctype.h>
#include "iniparser/src/iniparser.c"
#include "iniparser/src/dictionary.c"
#include "modules/modules.h"


int main(int argc, char **argv){
    int mRow=0, mCol=0,
        lOrR=0, cmd='!',
        inCy=1, numbOfBinds=0,
        numbOfFTypes=0,
        keys[ACTIONS];
    part lPart, rPart,
         *tmpCPt, *tmpAPt;
    char tEditor[32], term[64], loc[32];
    bnd **ubinds=NULL;
    fts **ftypes=NULL;
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;
    lPart.f.path = (char *) malloc(sizeof(char)*4096);
    rPart.f.path = (char *) malloc(sizeof(char)*4096);
    fTmp = (char *) malloc(sizeof(char)*8192);
    sTmp = (char *) malloc(sizeof(char)*8192);

    readConf(&lPart, &rPart, tEditor, term, keys, loc, &ubinds, &numbOfBinds, &ftypes, &numbOfFTypes);
    if (argc == 2)
        if (isValidPath(argv[1]))
            strcpy(lPart.f.path, argv[1]);
    if (!setlocale(LC_CTYPE, loc)){
        fprintf(stderr, "===\nCan't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf\nPress enter\n===\n");
        getchar();
    }
    initscr();
    noecho();
    keypad(stdscr, true);
    fillList(&lPart);
    fillList(&rPart);
    while (inCy){
        reSize(&lPart.w, &rPart.w, &mRow, &mCol);
        mvchgat(0, 0, -1, A_INVIS, 0, NULL);
        mvchgat(mRow-1, 0, -1, A_INVIS, 0, NULL);
        if (isalpha(cmd))
            mvprintw(0, mCol-1, "%c", cmd);
        tmpCPt = lOrR ? &rPart : &lPart;
        tmpAPt = lOrR ? &lPart : &rPart;
        refresh();
        process(tmpCPt, mRow, mCol, 1);
        process(tmpAPt, mRow, mCol, 0);
        move(mRow-1, 0);
        cmd = getch();
        if (cmd==KEY_UP)
            kUp(&tmpCPt->w, tmpCPt->f.numbOfLines, mRow);
        else if (cmd==KEY_DOWN)
            kDown(&tmpCPt->w, tmpCPt->f.numbOfLines, mRow);
        else if ((cmd==KEY_ENTER)||(cmd=='\n'))
            kEnter(tmpCPt, tEditor, term, mRow, ftypes, numbOfFTypes);
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
            strcpy(tmpCPt->f.path, tmpAPt->f.path);
            fillList(tmpCPt);
            setTop(&tmpCPt->w, mRow, 1);
            }
        else if (cmd==keys[4]){
            chDir(&tmpCPt->f, mRow);
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
            getInfo(tmpCPt, &tmpAPt->w, mCol, mRow);
        else if (cmd==keys[8])
            kCmd(tmpCPt, tmpAPt);
        else if (cmd==keys[9])
            kSearch(tmpCPt, mRow, mCol);
        else if (cmd==KEY_F(5)){
            fillList(&lPart);
            fillList(&rPart);
            setTop(&lPart.w, mRow, 1);
            setTop(&rPart.w, mRow, 1);
            }
        else if (cmd==KEY_F(6)){
            saveConf(&lPart, &rPart);
            endwin();
            aFree((void **)lPart.f.files, lPart.f.numbOfLines);
            aFree((void **)rPart.f.files, rPart.f.numbOfLines);
            while(numbOfBinds--){
                free(ubinds[numbOfBinds]->cmd);
                free(ubinds[numbOfBinds]);
            }
            free(ubinds);
            inCy=0;
            while(inCy<numbOfFTypes){
                free(ftypes[inCy]->filetype);
                if (ftypes[inCy]->lvl){
                    free(ftypes[inCy]->cmd);
                }
                ftypes[inCy]->cmd=NULL;
                free(ftypes[inCy++]);
            }
            free(ftypes);
            lPart.w.currentLine = 0; rPart.w.currentLine = 0;
            lPart.w.top = 0; rPart.w.top = 0;
            lPart.f.numbOfLines = 0; rPart.f.numbOfLines = 0;
            numbOfBinds = 0; numbOfFTypes = 0;
            readConf(&lPart, &rPart, tEditor, term, keys, loc, &ubinds, &numbOfBinds, &ftypes, &numbOfFTypes);
            if (!setlocale(LC_CTYPE, loc)){
                fprintf(stderr, "===\nCan't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf\nPress enter\n===\n");
                getchar();
            }
            initscr();
            noecho();
            keypad(stdscr, true);
            fillList(&lPart);
            fillList(&rPart);
            inCy=1;
            }
        else if (cmd==KEY_F(1))
            showCBinds(tmpAPt, mRow, ubinds, numbOfBinds);
        else if (cmd==KEY_HOME)
            setTop(&tmpCPt->w, mRow, 1);
        else if (cmd==KEY_END)
            kEnd(tmpCPt, mRow);
        else if (cmd==KEY_NPAGE)
            kPDown(tmpCPt, mRow);
        else if (cmd==KEY_PPAGE)
            kPUp(&tmpCPt->w, mRow);
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
    aFree((void **)lPart.f.files, lPart.f.numbOfLines);
    aFree((void **)rPart.f.files, rPart.f.numbOfLines);
    while(numbOfBinds--){
        free(ubinds[numbOfBinds]->cmd);
        free(ubinds[numbOfBinds]);
    }
    free(ubinds);
    inCy=0;
    while(inCy<numbOfFTypes){
        free(ftypes[inCy]->filetype);
        if (ftypes[inCy]->lvl){
            free(ftypes[inCy]->cmd);
        }
        ftypes[inCy]->cmd=NULL;
        free(ftypes[inCy++]);
    }
    free(ftypes);
    free(lPart.f.path);
    free(rPart.f.path);
    free(fTmp);
    free(sTmp);
    return 0;
}
