/*************
 it's a simple file manager (sfm)
 author  : illumin
 e-mail  : illumin.95@gmail.com
 date    : 2011-05-27
 version : 0,01b
*************/

#include <ncurses.h>
#include <dirent.h>
#include <string.h>

typedef struct{
        WINDOW *win;
        int currentLine;
        int top;
    } wnd;

typedef struct{
        char path[4096];
        char files[1024][1024];
        int numbOfLines;
    } fl;

typedef struct{
    wnd w;
    fl f;
} part;

//void readConf(part lp, rp)        omg...it smells like "int fd = open..."
//{
//}

void fillList(fl *f){
    DIR *fld;
    struct dirent *ent;

    f->numbOfLines=0;
    fld=opendir(f->path);
    strcpy(f->files[f->numbOfLines++], "../");
    while ((ent=readdir(fld)) != false)
        if ((strcmp(ent->d_name, ".") != 0)&&(strcmp(ent->d_name, "..") != 0)){
            strcpy(f->files[f->numbOfLines++], ent->d_name);
            if (ent->d_type == DT_DIR)
                strcat(f->files[f->numbOfLines-1], "/");
        }
    closedir(fld);
}

void draw(part *p, int mx){
    int tY=1;

    while ((tY<mx-3)&&(tY-1<=p->f.numbOfLines))
        mvwprintw(p->w.win, tY++, 1, "%s", p->f.files[p->w.top + tY-1]);
}

void kUp(wnd *w, int mx, int ml){

    --w->currentLine;
    if (w->currentLine<0){
        w->currentLine=mx-1;
        if (mx>ml-3)
            w->top=(mx-1)-(ml-5);
        else
            w->top=0;
    }
    if (w->currentLine<w->top)
        --w->top;

}

void kDown(wnd *w, int mx, int ml){

    ++w->currentLine;
    if (w->currentLine>mx-1){
        w->currentLine = 0;
        w->top=0;
    }
    if (w->currentLine>w->top+(ml-5))
        ++w->top;
}

void hl(wnd w, int mx){
    mvwchgat(w.win, w.currentLine-w.top+1, 1, mx/2-2, A_REVERSE, 0, NULL);
}

int main(int argc, char *argv[]){
    int mRow, mCol, lOrR=0, cmd;         //lOrR = left ot right (selected)
    part lPart, rPart;
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;

    initscr();
    noecho();
    getmaxyx(stdscr, mRow, mCol);
    keypad(stdscr, true);
    strcpy(lPart.f.path, "/home/illumin/Projects/C"); //TODO: use readConf
    strcpy(rPart.f.path, "/");
    fillList(&lPart.f);
    fillList(&rPart.f);
    lPart.w.win = newwin(mRow-2, mCol/2, 1, 0);
    rPart.w.win = newwin(mRow-2, mCol/2, 1, mCol/2);
    while (true){
        wclear(lPart.w.win);
        wclear(rPart.w.win);
        box(lPart.w.win, 0, 0);
        box(rPart.w.win, 0, 0);
        mvprintw(0, 0, "%s | %d/%d/%d", lPart.f.path, lPart.w.currentLine, lPart.w.top, lPart.f.numbOfLines);
        mvprintw(0, mCol/2, "%s | %d/%d/%d", rPart.f.path, rPart.w.currentLine, rPart.w.top, rPart.f.numbOfLines);
        draw(&lPart, mRow);
        draw(&rPart, mRow);
        lOrR ? hl(rPart.w, mCol) : hl(lPart.w, mCol);
        refresh();
        wrefresh(lPart.w.win);
        wrefresh(rPart.w.win);
        move(mRow-1, 0);

        cmd = getch();
        switch(cmd){
            case KEY_UP:
                lOrR ? kUp(&rPart.w, rPart.f.numbOfLines, mRow) : kUp(&lPart.w, lPart.f.numbOfLines, mRow);
                break;
            case KEY_DOWN:
                lOrR ? kDown(&rPart.w, rPart.f.numbOfLines, mRow) : kDown(&lPart.w, lPart.f.numbOfLines, mRow);
                break;
            case 9:
                lOrR = !lOrR;
                break;
            case 'q':
                endwin();
                return 0;
        }
    }

    endwin();
    return 0;
}


