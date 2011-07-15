/*************
 it's a simple file manager (sfm)
 author  : illumin
 e-mail  : illumin.95@gmail.com
 date    : 2011-05-29 => 2011-05-31
 version : 0.01c
*************/

#include <ncurses.h>
#include <dirent.h>
#include <string.h>

typedef struct{
        WINDOW *win;
        int currentLine;
        int top;
    } wnd;

typedef struct{                         //TODO: use wchar_t...
        char path[4096];
        char files[1024][1024];
        int numbOfLines;
        int hid;
    } fl;

typedef struct{
    wnd w;
    fl f;
} part;

//void readConf(part lp, rp)
//{
//}

void thLog(char *sCmd){
    int fd = open("log", 01, 0666);
    write(fd, sCmd, 100);
    close(fd);
}

void fillList(part *p){
    DIR *fld;
    struct dirent *ent;

    p->f.numbOfLines=0;
    p->w.currentLine=0;
    fld=opendir(p->f.path);
    while ((ent=readdir(fld)) != false)
        if ((strcmp(ent->d_name, ".") != 0)&&(strcmp(ent->d_name, "..") != 0))
            if (((!p->f.hid)&&(strncmp(ent->d_name, "..", 1)!=0))||(p->f.hid)){
                strcpy(p->f.files[p->f.numbOfLines++], ent->d_name);
                if (ent->d_type == DT_DIR)
                    strcat(p->f.files[p->f.numbOfLines-1], "/");
            }
    closedir(fld);
}

void draw(part p, int mx){
    int tY=1;

    while ((tY<mx-3)&&(tY-1<p.f.numbOfLines))
        mvwprintw(p.w.win, tY++, 1, "%s", p.f.files[p.w.top + tY-1]);
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

void kEnter(part *p){

    if (p->f.files[p->w.currentLine][strlen(p->f.files[p->w.currentLine])-1]=='/'){
        strcat(p->f.path, p->f.files[p->w.currentLine]);
        thLog(p->f.path);
    }
    else{
        //here will be opening with TEd
    }
}

void kPFld(fl *f){
    int i=0;

    for(i=strlen(f->path)-2;i>=0;i--)
        if (f->path[i]=='/'){
            f->path[i+1]='\0';
            break;
        }
}

void hl(wnd w, int mx){
    mvwchgat(w.win, w.currentLine-w.top+1, 1, mx/2-2, A_REVERSE, 0, NULL);
}

int main(int argc, char *argv[]){
    int mRow, mCol, lOrR=0, cmd;         //lOrR = left ot right (selected)
    part lPart, rPart;
    char tmp[1024], ttmp[1024];
    lPart.w.currentLine=0; rPart.w.currentLine=0;
    lPart.w.top=0; rPart.w.top=0;

    initscr();
    noecho();
    getmaxyx(stdscr, mRow, mCol);
    keypad(stdscr, true);
    if (argc == 2)
        strcpy(lPart.f.path, argv[1]);
    else
        strcpy(lPart.f.path, "/home/illumin/Projects/C/"); //TODO: use readConf
    strcpy(rPart.f.path, "/");
    lPart.f.hid=1; rPart.f.hid=1;                     //TODO: and it too
    fillList(&lPart);
    fillList(&rPart);
    lPart.w.win = newwin(mRow-2, mCol/2, 1, 0);
    rPart.w.win = newwin(mRow-2, mCol/2, 1, mCol/2);
    while (true){
        wclear(lPart.w.win);
        wclear(rPart.w.win);
        box(lPart.w.win, 0, 0);
        box(rPart.w.win, 0, 0);
        mvchgat(0, 0, -1, A_INVIS, 0, NULL);
        mvchgat(mRow-1, 0, -1, A_INVIS, 0, NULL);
        mvprintw(0, 0, "%s | %d/%d", lPart.f.path, lPart.w.currentLine, lPart.f.numbOfLines);
        mvprintw(0, mCol/2, "%s | %d/%d", rPart.f.path, rPart.w.currentLine, rPart.f.numbOfLines);
        if ((cmd>='A')&&(cmd<='z'))
            mvprintw(0, mCol-1, "%c", cmd);
        draw(lPart, mRow);
        draw(rPart, mRow);
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
            case '\n':      //interesting, but not 13, not KEY_ENTER...only '\n'
                lOrR ? kEnter(&rPart) : kEnter(&lPart);
                lOrR ? fillList(&rPart) : fillList(&lPart);
                break;
            case 9:
                lOrR = !lOrR;
                break;
            case 'q':
                endwin();
                return 0;
            case 'h':
                if (lOrR)
                    rPart.f.hid = !rPart.f.hid;
                else
                    lPart.f.hid = !lPart.f.hid;
                lOrR ? fillList(&rPart) : fillList(&lPart);
                break;
            case 's':
                lOrR ? strcpy(lPart.f.path, rPart.f.path) : strcpy(rPart.f.path, lPart.f.path);
                fillList(&lPart);
                fillList(&rPart);
                break;
            case 'c':
                echo();
                wscanw(stdscr, "%s", tmp);
                noecho();
                if (tmp[strlen(tmp)-1]!='/')
                    strcat(tmp, "/");
                if (tmp[0]=='/')
                    strcpy(lOrR ? rPart.f.path : lPart.f.path, tmp);
                else{
                    strcat(lOrR ? rPart.f.path : lPart.f.path, tmp);
                }
                fillList(&rPart);
                fillList(&lPart);
                break;
            case 'M':
                echo();
                wscanw(stdscr, "%s", tmp);
                noecho();
                if (tmp[0]=='/'){                          //FIXME: it must be simpler. without ttmp
                    strcpy(ttmp, "mkdir ");
                    system(strcat(ttmp, tmp));
                }
                else{
                    strcpy(ttmp, "mkdir ");
                    strcat(ttmp, lOrR ? rPart.f.path : lPart.f.path);
                    if (ttmp[strlen(ttmp)-1]!='/')
                        strcat(ttmp, "/");
                    strcat(ttmp, tmp);
                    system(ttmp);
                }
                fillList(&rPart);
                fillList(&lPart);
                break;
            case 'C':
                strcpy(tmp, "cp ");
                strcat(tmp, lOrR ? rPart.f.path : lPart.f.path);
                if (tmp[strlen(tmp)-1]!='/')
                    strcat(tmp, "/");
                strcat(tmp, lOrR ? rPart.f.files[rPart.w.currentLine] : lPart.f.files[lPart.w.currentLine]);
                if (tmp[strlen(tmp)-1]=='/')
                    strcat(tmp, " -r");
                strcat(tmp, " ");
                strcat(tmp, lOrR ? lPart.f.path : rPart.f.path);
                system(tmp);
                fillList(&lPart);
                fillList(&rPart);
                break;
            case 'm':
                strcpy(tmp, "mv ");
                strcat(tmp, lOrR ? rPart.f.path : lPart.f.path);
                strcat(tmp, lOrR ? rPart.f.files[rPart.w.currentLine] : lPart.f.files[lPart.w.currentLine]);
                strcat(tmp, " ");
                strcat(tmp, lOrR ? lPart.f.path : rPart.f.path);
                system(tmp);
                fillList(&lPart);
                fillList(&rPart);
                break;
            case 'r':
                strcpy(tmp, "rm -rf ");
                strcat(tmp, lOrR ? rPart.f.path : lPart.f.path);
                if (tmp[strlen(tmp)-1]!='/')
                    strcat(tmp, "/");
                strcat(tmp, lOrR ? rPart.f.files[rPart.w.currentLine] : lPart.f.files[lPart.w.currentLine]);
                system(tmp);
                fillList(&rPart);
                fillList(&lPart);
                break;
            case 'R':
                echo();
                wscanw(stdscr, "%s", tmp);
                noecho();
                strcpy(ttmp, "mv ");
                strcat(ttmp, lOrR ? rPart.f.path : lPart.f.path);
                strcat(ttmp, lOrR ? rPart.f.files[rPart.w.currentLine] : lPart.f.files[lPart.w.currentLine]);
                strcat(ttmp, " ");
                strcat(ttmp, lOrR ? rPart.f.path : lPart.f.path);
                strcat(ttmp, tmp);
                system(ttmp);
                fillList(&rPart);
                fillList(&lPart);
                break;
            case 'u':
                lOrR ? kPFld(&rPart.f) : kPFld(&lPart.f);
                lOrR ? fillList(&rPart) : fillList(&lPart);
                break;
        }
    }

    endwin();
    return 0;
}
