/*************
 it's a simple file manager (sfm)
 author  : illumin
 e-mail  : illumin.95@gmail.com
 date    : 2011-05-24 => 2011-05-27
 version : 0,01a
*************/

#include <ncurses.h>
#include <dirent.h>
#include <string.h>

#define HUETA 5

int main(int argc, char *argv[])
{
    WINDOW *lWin, *rWin;
    DIR *fld;
    struct dirent *ent;
    char lPath[4096], rPath[4096], tmp[1024];
    int mRow, mCol, curLeftRow=0, curRightRow=0,
        lOrR=0, cmd, tX, tY, sch=0;         //lOrR = left ot right (selected)
    struct fold {
        char lines[1024][1024];
        int numbOfLines;
        int topFringe;
        int bottomFringe;
    } lFold, rFold;
    lFold.numbOfLines=0; lFold.topFringe=0; lFold.bottomFringe=0;
    rFold.numbOfLines=0; rFold.topFringe=0; rFold.bottomFringe=0;
    //TODO: bool hidLeft, hidRight = hidden files in left\right part

    initscr();
    noecho();
    getmaxyx(stdscr, mRow, mCol);
    keypad(stdscr, TRUE);
    strcpy(lPath, "/home/illumin/Projects/C"); //TODO: reading from .cnf file
    strcpy(rPath, "/");
    fld=opendir(lPath);
    while ((ent=readdir(fld)) != false)
        if ((strcmp(ent->d_name, ".") != 0)&&(strcmp(ent->d_name, "..") != 0))
            strcpy(lFold.lines[lFold.numbOfLines++], ent->d_name);
    if (lFold.numbOfLines>mRow-4)
        lFold.bottomFringe=(mRow-5);
    else
        lFold.bottomFringe=lFold.numbOfLines;
    closedir(fld);
    fld=opendir(rPath);
    while ((ent=readdir(fld)) != false)
        if ((strcmp(ent->d_name, ".") != 0)&&(strcmp(ent->d_name, "..") != 0))
            strcpy(rFold.lines[rFold.numbOfLines++], ent->d_name);
    if (rFold.numbOfLines>mRow-4)
        rFold.bottomFringe=(mRow-5);
    else
        rFold.bottomFringe=rFold.numbOfLines;
    closedir(fld);
    lWin = newwin(mRow-2, mCol/2, 1, 0);
    rWin = newwin(mRow-2, mCol/2, 1, mCol/2);
    while (TRUE){
        wclear(lWin);
        wclear(rWin);
        mvprintw(0, 0, "%s | %d(%d/%d)", lPath, curLeftRow, lFold.topFringe, lFold.bottomFringe);
        mvprintw(0, mCol/2, "%s | %d(%d/%d)", rPath, curRightRow, rFold.topFringe, rFold.bottomFringe);
        box(lWin, 0, 0);
        box(rWin, 0, 0);

        tY=1; sch=0;
        if (lFold.topFringe == 0)
            mvwprintw(lWin, tY++, 1, "../");
        //for(sch=lFold.topFringe;sch<lFold.bottomFringe+tY-1-sch;sch++)
            //mvwprintw(lWin, tY++, 1, "%s", lFold.lines[sch]);
        while ((tY<mRow-4)&&(tY<lFold.numbOfLines))
            mvwprintw(lWin, tY++, 1, "%s", lFold.lines[sch++]);
        tY=1; sch=0;
        if (rFold.topFringe == 0)
            mvwprintw(rWin, tY++, 1, "../");
        //for(sch=rFold.topFringe;sch<rFold.bottomFringe+tY-1-sch;sch++)
        //   mvwprintw(rWin, tY++, 1, "%s", rFold.lines[sch]);
        while ((tY<mRow-4)&&(tY<rFold.numbOfLines))
            mvwprintw(rWin, tY++, 1, "%s", rFold.lines[sch++]);

        mvwchgat(lWin, curLeftRow-lFold.topFringe+1, 1, mCol/2-2, A_REVERSE, 0, NULL);
        mvwchgat(rWin, curRightRow-rFold.topFringe+1, 1, mCol/2-2, A_REVERSE, 0, NULL);
        refresh();
        wrefresh(lWin);
        wrefresh(rWin);
        move(mRow-1, 0);
        cmd = getch();
        switch(cmd){
            case KEY_UP:
                if (lOrR){
                    --curRightRow;
                    if (curRightRow<0){
                        curRightRow=rFold.numbOfLines;
                        rFold.bottomFringe = rFold.numbOfLines;
                        if (rFold.numbOfLines>mRow-HUETA)
                            rFold.topFringe=rFold.bottomFringe-(mRow-HUETA);
                        else
                            rFold.topFringe=0;
                    }
                    if (curRightRow<rFold.topFringe){
                        --rFold.topFringe;
                        --rFold.bottomFringe;
                    }
                }
                else{
                    --curLeftRow;
                    if (curLeftRow<0){
                        curLeftRow=lFold.numbOfLines;
                        lFold.bottomFringe = lFold.numbOfLines;
                        if (lFold.numbOfLines>mRow-HUETA)
                            lFold.topFringe=lFold.bottomFringe-(mRow-HUETA);
                        else
                            lFold.topFringe=0;
                    }
                    if (curLeftRow<lFold.topFringe){
                        --lFold.topFringe;
                        --lFold.bottomFringe;
                    }
                }
                break;
            case KEY_DOWN:
                if (lOrR){
                    ++curRightRow;
                    if (curRightRow<0){
                        curRightRow=rFold.numbOfLines;
                        rFold.bottomFringe = rFold.numbOfLines;
                        if (rFold.numbOfLines>mRow-HUETA)
                            rFold.topFringe=rFold.bottomFringe-(mRow-HUETA);
                        else
                            rFold.topFringe=0;
                    }
                    if (curRightRow<rFold.topFringe){
                        --rFold.topFringe;
                        --rFold.bottomFringe;
                    }
                }
                else{
                    --curLeftRow;
                    if (curLeftRow<0){
                        curLeftRow=lFold.numbOfLines;
                        lFold.bottomFringe = lFold.numbOfLines;
                        if (lFold.numbOfLines>mRow-HUETA)
                            lFold.topFringe=lFold.bottomFringe-(mRow-HUETA);
                        else
                            lFold.topFringe=0;
                    }
                    if (curLeftRow<lFold.topFringe){
                        --lFold.topFringe;
                        --lFold.bottomFringe;
                    }
                }
                break;
            case 9:
                lOrR = !lOrR;
                break;
            case 'q':
                //TODO: save settings (lPath, rPath, maybe smth else)
                endwin();
                return 0;
            case 'M':
                echo();
                scanw("%s", tmp);
                //system("mkdir "+nPath+tmp)
                noecho();
                break;
            //case KEY_ENTER:
                //blah-bla-blah strcat path folder[currentRow]
            //TODO: make user-setable actions to F1-F12 (like "F1: go to /media", "F2: open selected with geany", etc)
            //TODO: case 'm' = mv
            //TODO: case 'c' = cp
            //TODO: case 'r' = rm -rf
        }
    }

    endwin();
    return 0;
}
