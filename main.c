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
#include <stddef.h>
#include <string.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include <ctype.h>
#include <pwd.h>

#include "iniparser/src/iniparser.h"

#define ACTIONS 10

static const char* LOCALE_ERROR_MESSAGE =
		"===\nCan't set the locale!\nSome filenames can display wrong.\nCheck ~/.config/sfm/sfm.conf\nPress enter\n===\n";

typedef struct {
	WINDOW *win;
	unsigned int currentLine;
	unsigned int top;
} sfmWindowStruct;

typedef struct {
	char path[4096];
	struct dirent **files;
	unsigned int numbOfFiles;
	unsigned char showHidden;
} sfmFolderStruct;

typedef struct {
	sfmWindowStruct sWind;
	sfmFolderStruct sFold;
} sfmPart;

typedef struct {
	char key;
	char *cmd;
} sfmBind;

typedef struct {
	char *filetype;
	char *cmd;
	unsigned char lvl;
} sfmFiletype;

/* Глобальные темповые переменные. */
char fTmp[8192], sTmp[8192];

/*
 * Освобождает память, выделенную под len элеметов ptr.
 */

void aFree(void **ptr, size_t len);

/*
 * Принимает 2 параметра.
 *  Первый выводится в качестве вопроса
 *  Во второй записывается ответ.
 *   При отсутствии ответа возвращается пустая строка ("\0").
 */

void ask(char *i, char *o);

/*
 * Запрашивает название директории и переходит в нее.
 * Если введенная строка начинается с '/', то путь рассматривается как абсолютный.
 * Гарантирует, что part->sFold.path[ strlen( part->sFold.path ) - 1 ] == '/'.
 */

void chDir(sfmFolderStruct *folder, int mRow);

/*
 * Отрисовывает в part->sWind.win содержимое part->sFold.files, начиная с top, заканчивая top + mRow - 4.
 * Если элемент является директорией, в конец дописывается '/'
 *                       исполняемым файлом     -          '*'
 *                       ссылкой                -          '>'
 *                       символьным устройством -          '@'
 *                       блочным устройством    -          '#'
 * Название элемента обрезается до mCol/2-3. mCol/2 - половина всего (1 окно). -2 для бордеров
 и -1 для идентификатора (дир, ссылка, etc).
 */

void draw(sfmPart *part, int mRow, int mCol);

/*
 * Дополняет строку cmd так, чтобы весь вывод шел в /dev/null
 */

void executeInBackground(char *cmd);

/*
 * Если под part->sFold.files была выделена память, освобождает ее.
 * Считывает директорию, путь к которой находится в part->sFold.path,
 *  помещает результат в part->sFold.files,
 *  использует селектор slt, если part->sFold.showHidden = 0,
 *  сортирует в алфавитном порядке.
 */

void fillList(sfmPart *part);

/*
 * Принимает ссылку на ссылку на ссылку (сслыку на массив ссылок) типа sfmFiletype, ссылку на numbOfFTypes и
 *  ссылку на словарь ini, связанный с конфигурационным файлом.
 * Находит по словарю значения, указанные в секции Filetypes конф. файла, запиывает пару "расширение файла - программа"
 *  в ftypes и увеличивает numbOfFTypes на 1.
 * Если программа уже находится в ftypes, память не выделяется.
 */

void getFileTypes(sfmFiletype ***ftypes, int *numbOfFTypes, dictionary *ini);

/*
 * Выводит информацию по текущему элементу:
 *  полное имя
 *  размер
 *  права доступа
 *  владельца
 */

void getInfo(sfmPart *part, sfmWindowStruct *wind, int mCol, int mRow);

/*
 * Принимает ссылку на ссылку на ссылку (сслыку на массив ссылок) типа bnd, ссылку на numbOfBinds и
 *  ссылку на словарь ini, связанный с конфигурационным файлом.
 * Находит по словарю значения, указанные в секции uBinds конф. файла, запиывает пару "клавиша - команда"
 *  в ubinds и увеличивает numbOfBinds на 1.
 */

void getUserBinds(sfmBind ***ubinds, int *numbOfBinds, dictionary *ini);

/*
 * Запрашивает строку для перехода и переходит на нее.
 * Если она < 0 или > numbOfLines, обнуляет currentLine.
 */

void goToLine(sfmPart *part, int mRow);

/*
 * Проверяет, является ли строка путем к директории.
 */

int isValidPath(char *path);

/*
 * Запрашивает команду на выполнение, если в ней найден символ '%', отправляет в parseCmd, иначе просто выполняет.
 */

void kCmd(sfmPart *fPart, sfmPart *sPart);

/*
 * Обрабатывает нажатие Enter.
 * Если текущий элемент - директория, переходит в нее,
 *                        бинарник - выполняет в терминале,
 * иначе - ищет в ftypes команду для этого типа файла,
 * если там такой нет, пробует открыть текстовым редактором.
 */

void kEnter(sfmPart *part, char *tEditor, char *term, int mRow,
		sfmFiletype **ftypes, int numbOfFTypes);

/*
 * Переходит в верхнюю директорию.
 *  = Обрезает путь по последнему '/'.
 */

void kPFld(sfmFolderStruct *folder);

/*
 * Считывает символы до первого непечатного, каждый раз устанавливая currentLine на элемент, максимально соответствующий
 *  введенным символам.
 */

void kSearch(sfmPart *part, int mRow, int mCol);

/*
 * Сдвигает currentLine на 1 вверх.
 */

void kUp(sfmWindowStruct *wind, int numbOfLines, int mRow);

/*
 * Сдвигает currentLine на 1 вниз.
 */

void kDown(sfmWindowStruct *wind, int numbOfLines, int mRow);

/*
 * Устанавливает currentLine на последний элемент.
 */

void kEnd(sfmPart *part, int mRow);

/*
 * Поднимает currentLine на "страницу" (mRow-4 элемента).
 */

void kPUp(sfmWindowStruct *wind, int mRow);

/*
 * Опускает currentLine на "страницу" (mRow-4 элемента).
 */

void kPDown(sfmPart *part, int mRow);

/*
 * Выводит строку состояния - путь до текущей директории, текущее положение и кол-во элементов в директории.
 */

void nfo(sfmPart *part, int mCol);

/*
 * Заменяет вхождения %cf, %cF, %af в cmd на путь к текущей директории, название текущего файла и
 *  путь к неактивной директории соответственно.
 * %rc, %ra - обновляют содержимое директории в активном и неактивном окне соответствено.
 */

void parseCmd(sfmPart *fPart, sfmPart *sPart, char *cmd);

/*
 * Получает на вход кол-во байт и возвращает строку с числом, кратным исходному, и приставкой.
 */

char *parseSize(long int s);

/*
 * Абстракция над ф-циями для прорисовки интерфейса
 */

void process(sfmPart *part, int mRow, int mCol, int showNfo);

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

void readConf(sfmPart *fPart, sfmPart *sPart, char *tEditor, char *term,
		int *keys, char *loc, sfmBind ***ubinds, int *numbOfBinds,
		sfmFiletype ***ftypes, int *numbOfFTypes);

/*
 * При несоответствии реальных размеров окна с заявленными, создает новые окна с реальными размерами.
 */

void reSize(sfmWindowStruct *fWind, sfmWindowStruct *sWind, int *mRow,
		int *mCol);

/*
 * Сохраняет путь левой и правой части окна, настройки видимости.
 */

void saveConf(sfmPart *fPart, sfmPart *sPart);

/*
 * Принимает указатель на файловую переменную, связанную с конфигурационный файлом, открытым для записи,
 *  и массив строк с названиями действий, которые нужно связать с клавишами.
 * Кроме действий из actions запрашивает название текстового редактора и эмулятора терминала.
 * Устанавливает локаль по $LANG.
 */

void setKeys(FILE *f, const char **actions);

/*
 * Устанавливает top текущего окна в зависимости от currentLine и mRow (максимального кол-ва строк окна).
 * Если mkSt != 0, обнуляет текущее положение и top.
 */

void setTop(sfmWindowStruct *wind, int mRow, int mkSt);

/*
 * Выводит список "клавиша - бинд" из ubinds.
 */

void showCBinds(sfmPart *part, int mRow, sfmBind **ubinds, int numbOfBinds);

/*
 * Селектор скрытых файлов.
 */

int slt(const struct dirent *d);

/* Реализация */

int main(int argc, char **argv) {
	int mRow = 0, mCol = 0, lOrR = 0, cmd = '!', inCy = 1, numbOfBinds = 0,
			numbOfFTypes = 0, keys[ACTIONS];
	sfmPart lPart, rPart, *tmpCPt, *tmpAPt;
	char tEditor[32], term[64], loc[32];
	sfmBind **ubinds = NULL;
	sfmFiletype **ftypes = NULL;

	lPart.sWind.currentLine = 0;
	lPart.sFold.numbOfFiles = 0;
	lPart.sWind.top = 0;

	rPart.sWind.currentLine = 0;
	rPart.sFold.numbOfFiles = 0;
	rPart.sWind.top = 0;

	readConf(&lPart, &rPart, tEditor, term, keys, loc, &ubinds, &numbOfBinds,
			&ftypes, &numbOfFTypes);
	if (argc == 2)
		if (isValidPath(argv[1]))
			strcpy(lPart.sFold.path, argv[1]);
	if (!setlocale(LC_ALL, loc)) {
		fprintf(stderr, LOCALE_ERROR_MESSAGE);
		getchar();
	}

	initscr();
	noecho();
	keypad(stdscr, true);
	fillList(&lPart);
	fillList(&rPart);

	while (inCy) {
		reSize(&lPart.sWind, &rPart.sWind, &mRow, &mCol);
		mvchgat(0, 0, -1, A_INVIS, 0, NULL);
		mvchgat(mRow - 1, 0, -1, A_INVIS, 0, NULL);
		if (isalpha(cmd))
			mvprintw(0, mCol - 1, "%c", cmd);
		tmpCPt = lOrR ? &rPart : &lPart;
		tmpAPt = lOrR ? &lPart : &rPart;
		refresh();
		process(tmpCPt, mRow, mCol, 1);
		process(tmpAPt, mRow, mCol, 0);
		move(mRow - 1, 0);
		cmd = getch();
		if (cmd == KEY_UP)
			kUp(&tmpCPt->sWind, tmpCPt->sFold.numbOfFiles, mRow);
		else if (cmd == KEY_DOWN)
			kDown(&tmpCPt->sWind, tmpCPt->sFold.numbOfFiles, mRow);
		else if ((cmd == KEY_ENTER) || (cmd == '\n')) {
			kEnter(tmpCPt, tEditor, term, mRow, ftypes, numbOfFTypes);
			chdir(tmpCPt->sFold.path);
		} else if (cmd == keys[0]) {
			lOrR = !lOrR;
			chdir(tmpCPt->sFold.path);
		} else if (cmd == keys[1]) {
			ask("Are you sure, quit?", fTmp);
			if (fTmp[0] == 'y')
				inCy = 0;
		} else if (cmd == keys[2]) {
			tmpCPt->sFold.showHidden = !tmpCPt->sFold.showHidden;
			fillList(tmpCPt);
			setTop(&tmpCPt->sWind, mRow, 1);
		} else if (cmd == keys[3]) {
			strcpy(tmpCPt->sFold.path, tmpAPt->sFold.path);
			fillList(tmpCPt);
			setTop(&tmpCPt->sWind, mRow, 1);
			chdir(tmpCPt->sFold.path);
		} else if (cmd == keys[4]) {
			chDir(&tmpCPt->sFold, mRow);
			fillList(tmpCPt);
			setTop(&tmpCPt->sWind, mRow, 1);
			chdir(tmpCPt->sFold.path);
		} else if (cmd == keys[5]) {
			kPFld(&tmpCPt->sFold);
			fillList(tmpCPt);
			setTop(&tmpCPt->sWind, mRow, 1);
			chdir(tmpCPt->sFold.path);
		} else if (cmd == keys[6])
			goToLine(tmpCPt, mRow);
		else if (cmd == keys[7])
			getInfo(tmpCPt, &tmpAPt->sWind, mCol, mRow);
		else if (cmd == keys[8])
			kCmd(tmpCPt, tmpAPt);
		else if (cmd == keys[9])
			kSearch(tmpCPt, mRow, mCol);
		else if (cmd == KEY_F(5)) {
			fillList(&lPart);
			fillList(&rPart);
			setTop(&lPart.sWind, mRow, 1);
			setTop(&rPart.sWind, mRow, 1);
		} else if (cmd == KEY_F(6)) {
			saveConf(&lPart, &rPart);
			endwin();
			aFree((void **) lPart.sFold.files, lPart.sFold.numbOfFiles);
			aFree((void **) rPart.sFold.files, rPart.sFold.numbOfFiles);
			while (numbOfBinds--) {
				free(ubinds[numbOfBinds]->cmd);
				free(ubinds[numbOfBinds]);
			}
			free(ubinds);
			inCy = 0;
			while (inCy < numbOfFTypes) {
				free(ftypes[inCy]->filetype);
				if (ftypes[inCy]->lvl) {
					free(ftypes[inCy]->cmd);
				}
				ftypes[inCy]->cmd = NULL;
				free(ftypes[inCy++]);
			}
			free(ftypes);
			lPart.sWind.currentLine = 0;
			rPart.sWind.currentLine = 0;
			lPart.sWind.top = 0;
			rPart.sWind.top = 0;
			lPart.sFold.numbOfFiles = 0;
			rPart.sFold.numbOfFiles = 0;
			numbOfBinds = 0;
			numbOfFTypes = 0;
			readConf(&lPart, &rPart, tEditor, term, keys, loc, &ubinds,
					&numbOfBinds, &ftypes, &numbOfFTypes);
			if (!setlocale(LC_ALL, loc)) {
				fprintf(stderr, LOCALE_ERROR_MESSAGE);
				getchar();
			}
			initscr();
			noecho();
			keypad(stdscr, true);
			fillList(&lPart);
			fillList(&rPart);
			inCy = 1;
		} else if (cmd == KEY_F(1))
			showCBinds(tmpAPt, mRow, ubinds, numbOfBinds);
		else if (cmd == KEY_HOME)
			setTop(&tmpCPt->sWind, mRow, 1);
		else if (cmd == KEY_END)
			kEnd(tmpCPt, mRow);
		else if (cmd == KEY_NPAGE)
			kPDown(tmpCPt, mRow);
		else if (cmd == KEY_PPAGE)
			kPUp(&tmpCPt->sWind, mRow);
		else {
			for (inCy = 0; inCy < numbOfBinds; inCy++)
				if (cmd == ubinds[inCy]->key) {
					parseCmd(tmpCPt, tmpAPt, ubinds[inCy]->cmd);
					break;
				}
			inCy = 1;
		}
	}

	endwin();
	saveConf(&lPart, &rPart);
	aFree((void **) lPart.sFold.files, lPart.sFold.numbOfFiles);
	aFree((void **) rPart.sFold.files, rPart.sFold.numbOfFiles);
	while (numbOfBinds--) {
		free(ubinds[numbOfBinds]->cmd);
		free(ubinds[numbOfBinds]);
	}
	free(ubinds);
	inCy = 0;
	while (inCy < numbOfFTypes) {
		free(ftypes[inCy]->filetype);
		if (ftypes[inCy]->lvl) {
			free(ftypes[inCy]->cmd);
		}
		ftypes[inCy]->cmd = NULL;
		free(ftypes[inCy++]);
	}
	free(ftypes);
	return 0;
}

void aFree(void **ptr, size_t len) {
	while (len--)
		free(ptr[len]);

	free(ptr);
}

void ask(char *i, char *o) {
	echo();
	printw("%s: ", i);
	if (scanw("%[^\n]", o) == -1)
		strcpy(o, "");
	noecho();
}

void chDir(sfmFolderStruct *folder, int mRow) {
	size_t ln;

	ask("Name of folder", fTmp);
	if (strcmp(fTmp, "")) {
		if (fTmp[(ln = strlen(fTmp)) - 1] != '/')
			fTmp[ln] = '/', fTmp[ln + 1] = '\0';
		strcpy(sTmp, folder->path);
		if (isValidPath((fTmp[0] == '/') ? fTmp : strcat(folder->path, fTmp))) {
			if (fTmp[0] == '/')
				strcpy(folder->path, fTmp);
		} else {
			if (fTmp[0] != '/')
				strcpy(folder->path, sTmp);
			mvchgat(mRow - 1, 0, -1, A_INVIS, 0, NULL);
			printw("Wrong name of folder");
			getch();
		}
	}
}

void draw(sfmPart *part, int mRow, int mCol) {
	int tY = 0;
	struct stat fStat;

	while ((tY < mRow - 4) && (tY < part->sFold.numbOfFiles)) {
		mvwprintw(part->sWind.win, tY + 1, 1, "%.*s", mCol / 2 - 3,
				part->sFold.files[part->sWind.top + tY]->d_name);
		sprintf(fTmp, "%s%s", part->sFold.path,
				part->sFold.files[part->sWind.top + tY++]->d_name);
		stat(fTmp, &fStat);
		if (S_ISDIR(fStat.st_mode))
			wprintw(part->sWind.win, "/");
		else if (fStat.st_mode & S_IXUSR)
			wprintw(part->sWind.win, "*");
		else if (S_ISLNK(fStat.st_mode))
			wprintw(part->sWind.win, ">");
		else if (S_ISCHR(fStat.st_mode))
			wprintw(part->sWind.win, "@");
		else if (S_ISBLK(fStat.st_mode))
			wprintw(part->sWind.win, "#");
	}
}

void executeInBackground(char *cmd) {
	strcat(cmd, " > /dev/null 2> /dev/null &");
	system(cmd);
}

void fillList(sfmPart *part) {
	if (part->sFold.numbOfFiles)
		aFree((void **) part->sFold.files, part->sFold.numbOfFiles);
	part->sFold.numbOfFiles = scandir(part->sFold.path, &part->sFold.files,
			(part->sFold.showHidden) ? 0 : slt, alphasort);
}

void getFileTypes(sfmFiletype ***ftypes, int *numbOfFTypes, dictionary *ini) {
	char *tmp;
	int i;
	*ftypes = (sfmFiletype **) malloc(sizeof(sfmFiletype *));

	strcpy(fTmp, iniparser_getstring(ini, "filetypes:types", NULL ));
	tmp = strtok(fTmp, " ");
	while (tmp) {
		sprintf(sTmp, "filetypes:%s", tmp);
		strcpy(sTmp, iniparser_getstring(ini, sTmp, NULL ));
		if ((*numbOfFTypes) > 0)
			*ftypes = (sfmFiletype **) realloc(*ftypes,
					sizeof(sfmFiletype *) * ((*numbOfFTypes) + 1));
		(*ftypes)[*numbOfFTypes] = (sfmFiletype *) malloc(sizeof(sfmFiletype));
		(*ftypes)[*numbOfFTypes]->filetype = (char *) malloc(strlen(tmp) + 1);
		strcpy((*ftypes)[*numbOfFTypes]->filetype, tmp);
		for (i = 0; i < (*numbOfFTypes); i++)
			if ((*ftypes)[i]->lvl)
				if (!strcmp((*ftypes)[i]->cmd, sTmp)) {
					(*ftypes)[*numbOfFTypes]->cmd = (*ftypes)[i]->cmd;
					(*ftypes)[*numbOfFTypes]->lvl = 0;
					i = 0;
					break;
				}
		if (i || (!(*numbOfFTypes))) {
			(*ftypes)[*numbOfFTypes]->cmd = (char *) malloc(sizeof(sTmp) + 1);
			strcpy((*ftypes)[*numbOfFTypes]->cmd, sTmp);
			(*ftypes)[*numbOfFTypes]->lvl = 1;
		}
		++(*numbOfFTypes);
		tmp = strtok(NULL, " ");
	}
}

void getInfo(sfmPart *part, sfmWindowStruct *wind, int mCol, int mRow) {
	const short int right[9] = { 0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002,
			0001 };
	const char *rwx = "rwx";
	int tY = 1, i;
	struct stat fStat;
	struct passwd *un;

	wclear(wind->win);
	box(wind->win, 0, 0);
	sprintf(fTmp, "%s%s", part->sFold.path,
			part->sFold.files[part->sWind.currentLine]->d_name);
	stat(fTmp, &fStat);
	mvwprintw(wind->win, tY, 1, "Name: %s",
			part->sFold.files[part->sWind.currentLine]->d_name);
	tY += (strlen(part->sFold.files[part->sWind.currentLine]->d_name) + 6)
			/ (mCol / 2) + 1;
	mvwprintw(wind->win, tY++, 1, "Size: %s", parseSize(fStat.st_size));
	mvwprintw(wind->win, tY++, 1, "Access mode: ");
	for (i = 0; i < 9; i++) {
		wprintw(wind->win, "%c", (fStat.st_mode & right[i]) ? rwx[i % 3] : '-');
		((i + 1) % 3 == 0) ? wprintw(wind->win, " ") : 0;
	}
	wprintw(wind->win, "(%o)",
			(fStat.st_mode & S_IRWXU)+(fStat.st_mode & S_IRWXG)+(fStat.st_mode & S_IRWXO));
	un = getpwuid(fStat.st_uid);
	mvwprintw(wind->win, tY++, 1, "Owner: %s", un->pw_name);
	move(mRow-1, 0);
	wrefresh(wind->win);
	getch();
}

void getUserBinds(sfmBind ***ubinds, int *numbOfBinds, dictionary *ini) {
	const char *symbols = "qwertyuiopasdfghjklzxcvbnm";
	int i;
	*ubinds = (sfmBind **) malloc(sizeof(sfmBind *));

	for (i = 0; i < 26; i++) {
		sprintf(fTmp, "ubinds:%c", symbols[i]);
		strcpy(fTmp, iniparser_getstring(ini, fTmp, ""));
		if (fTmp[0] != '\0') {
			if ((*numbOfBinds) > 0)
				*ubinds = (sfmBind **) realloc(*ubinds,
						sizeof(sfmBind *) * ((*numbOfBinds) + 1));
			(*ubinds)[*numbOfBinds] = (sfmBind *) malloc(sizeof(sfmBind));
			(*ubinds)[*numbOfBinds]->key = symbols[i];
			(*ubinds)[*numbOfBinds]->cmd = (char *) malloc(
					(size_t) strlen(fTmp) + 1);
			strcpy((*ubinds)[*numbOfBinds]->cmd, fTmp);
			++(*numbOfBinds);
		}
	}
}

void goToLine(sfmPart *part, int mRow) {
	echo();
	printw("New position: ");
	scanw("%d", &part->sWind.currentLine);
	noecho();
	if ((--part->sWind.currentLine < 0)
			|| (part->sWind.currentLine > part->sFold.numbOfFiles)) {
		part->sWind.top = 0;
		part->sWind.currentLine = 0;
	} else
		setTop(&part->sWind, mRow, 0);
}

int isValidPath(char *path) {
	struct stat fStat;

	stat(path, &fStat);
	return S_ISDIR(fStat.st_mode);
}

void kCmd(sfmPart *fPart, sfmPart *sPart) {
	ask("Command", fTmp);
	if (strcmp(fTmp, "")) {
		if (strchr(fTmp, '%'))
			parseCmd(fPart, sPart, fTmp);
		else
			system(fTmp);
	}
}

void kEnter(sfmPart *part, char *tEditor, char *term, int mRow,
		sfmFiletype **ftypes, int numbOfFTypes) {
	int i = 0;
	char *tmp;
	struct stat fStat;

	if (part->sFold.numbOfFiles) {
		sprintf(fTmp, "%s%s", part->sFold.path,
				part->sFold.files[part->sWind.currentLine]->d_name);
		stat(fTmp, &fStat);
		if (S_ISDIR(fStat.st_mode)) {
			strcat(part->sFold.path,
					part->sFold.files[part->sWind.currentLine]->d_name);
			strcat(part->sFold.path, "/");
			fillList(part);
			setTop(&part->sWind, mRow, 1);
		} else if (S_ISREG(fStat.st_mode)) {
			if (fStat.st_mode & S_IXUSR)
				sprintf(sTmp, "%s \'%s\'", term, fTmp);
			else {
				sprintf(sTmp, "%s \'%s\'", tEditor, fTmp);
				tmp = &fTmp[strlen(fTmp) - 1];
				while (((*(tmp - 1)) != '.') && (tmp != &fTmp[0]))
					--tmp;
				for (; i < numbOfFTypes; i++)
					if (!strcmp(ftypes[i]->filetype, tmp)) {
						sprintf(sTmp, "%s \'%s\'", ftypes[i]->cmd, fTmp);
						break;
					}
			}
			executeInBackground(sTmp);
		}
	}
}

void kPFld(sfmFolderStruct *folder) {
	int i;

	for (i = strlen(folder->path) - 2; i >= 0; i--)
		if (folder->path[i] == '/') {
			folder->path[i + 1] = '\0';
			break;
		}
}

void kSearch(sfmPart *part, int mRow, int mCol) {
	int i = 0, key, pos = 0;

	echo();
	printw("/");
	while (isprint((key = getch()))) {
		sTmp[i++] = key;
		sTmp[i] = '\0';
		for (; pos < part->sFold.numbOfFiles; pos++)
			if (!strncmp(part->sFold.files[pos]->d_name, sTmp, i)) {
				part->sWind.currentLine = pos;
				setTop(&part->sWind, mRow, 0);
				process(part, mRow, mCol, 1);
				move(mRow - 1, i + 1);
				break;
			}
	}
	noecho();
}

void kUp(sfmWindowStruct *wind, int numbOfLines, int mRow) {
	--wind->currentLine;
	if (wind->currentLine < 0) {
		wind->currentLine = numbOfLines - 1;
		setTop(wind, mRow, 0);
	}
	if (wind->currentLine < wind->top)
		--wind->top;
}

void kDown(sfmWindowStruct *wind, int numbOfLines, int mRow) {
	++wind->currentLine;
	if (wind->currentLine > numbOfLines - 1)
		setTop(wind, mRow, 1);
	if (wind->currentLine > wind->top + (mRow - 5))
		++wind->top;
}

void kEnd(sfmPart *part, int mRow) {
	part->sWind.currentLine = part->sFold.numbOfFiles - 1;
	setTop(&part->sWind, mRow, 0);
}

void kPUp(sfmWindowStruct *wind, int mRow) {
	if ((wind->currentLine - (mRow - 4)) > 0) {
		wind->currentLine = wind->currentLine - (mRow - 4);
		setTop(wind, mRow, 0);
	} else
		setTop(wind, mRow, 1);
}

void kPDown(sfmPart *part, int mRow) {
	if ((part->sWind.currentLine + (mRow - 4)) < part->sFold.numbOfFiles) {
		part->sWind.currentLine = part->sWind.currentLine + (mRow - 4);
		setTop(&part->sWind, mRow, 0);
	} else
		kEnd(part, mRow);
}

void nfo(sfmPart *part, int mCol) {
	mvwchgat(part->sWind.win, part->sWind.currentLine - part->sWind.top + 1, 1,
			mCol / 2 - 2, A_REVERSE, 0, NULL);
	if (strlen(part->sFold.path) < (mCol - 15))
		mvprintw(0, 0, "%s", part->sFold.path);
	else {
		mvprintw(0, 0, "..");
		printw("%s", part->sFold.path[strlen(part->sFold.path) - mCol + 17]);
	}
	mvprintw(0, mCol - 15, "|%d/%d", part->sWind.currentLine + 1,
			part->sFold.numbOfFiles);
}

void parseCmd(sfmPart *fPart, sfmPart *sPart, char *cmd) {
	char *outCmd = (char *) malloc(sizeof(char) * 1024), *tmp;
	sfmPart *rf = NULL;

	strcpy(fTmp, cmd);
	strcpy(outCmd, "");
	tmp = strtok(fTmp, "%");
	while (tmp) {
		if (tmp[0] == 'U') {
			++tmp;
			ask(tmp, sTmp);
			if (!strcmp(sTmp, ""))
				return;
			strcat(outCmd, sTmp);
		} else if (((tmp[0] == 'c') || (tmp[0] == 'a') || (tmp[0] == 'r'))
				&& ((tmp[1] == 'f') || (tmp[1] == 'F') || (tmp[1] == 'c')
						|| (tmp[1] == 'a'))) {
			if ((tmp[0] == 'c') && (tmp[1] == 'f'))
				strcat(outCmd, fPart->sFold.path);
			else if ((tmp[0] == 'c') && (tmp[1] == 'F'))
				strcat(outCmd,
						fPart->sFold.files[fPart->sWind.currentLine]->d_name);
			else if (tmp[0] == 'a' && (tmp[1] == 'f'))
				strcat(outCmd, sPart->sFold.path);
			else if ((tmp[0] == 'r') && ((tmp[1] == 'c') || (tmp[1] == 'a')))
				rf = (tmp[1] == 'c') ? fPart : sPart;
			tmp += 2;
			tmp ? strcat(outCmd, tmp) : 0;
		} else
			strcat(outCmd, tmp);
		tmp = strtok(NULL, "%");
	}
	system(outCmd);
	if (rf)
		fillList(rf);

	free(tmp);
	free(outCmd);
}

char *parseSize(long int s) {
	const char *sizes[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB" };
	int i = 0;
	double sz = (double) s;

	while (sz > 1024) {
		sz /= 1024;
		++i;
	}
	sprintf(fTmp, "%.2f %s", sz, sizes[i]);

	return fTmp;
}

void process(sfmPart *part, int mRow, int mCol, int showNfo) {
	wclear(part->sWind.win);
	box(part->sWind.win, 0, 0);
	draw(part, mRow, mCol);
	if (showNfo)
		nfo(part, mCol);
	wrefresh(part->sWind.win);
}

void readConf(sfmPart *fPart, sfmPart *sPart, char *tEditor, char *term,
		int *keys, char *loc, sfmBind ***ubinds, int *numbOfBinds,
		sfmFiletype ***ftypes, int *numbOfFTypes) {
	const char *actions[ACTIONS] = { "Another window", "Quit", "Hidden mode",
			"Same folder", "Change folder", "Parent folder", "Go to line",
			"Info", "Execute command", "Search" };
	int i = 0;
	FILE *f;
	dictionary *ini;

	initscr();
	sprintf(fTmp, "%s/.config/sfm/tmp", getenv("HOME"));
	if (!(f = fopen(fTmp, "r"))) {
		system("mkdir -p ~/.config/sfm/");
		f = fopen(fTmp, "w");
		fprintf(f,
				"[Main]\nlPath = / ;\nrPath = / ;\nlHid = 0 ;\nrHid = 0 ;\n");
	}
	fclose(f);
	ini = iniparser_load(fTmp);
	strcpy(fPart->sFold.path, iniparser_getstring(ini, "main:lpath", "/"));
	strcpy(sPart->sFold.path, iniparser_getstring(ini, "main:rpath", "/"));
	fPart->sFold.showHidden = iniparser_getint(ini, "main:lhid", 0);
	sPart->sFold.showHidden = iniparser_getint(ini, "main:rhid", 0);
	iniparser_freedict(ini);
	sprintf(fTmp, "%s/.config/sfm/sfm.conf", getenv("HOME"));
	if (!(f = fopen(fTmp, "r"))) {
		printw("Main .conf file not found. (C)onfigure now or use (d)efault?");
		i = getch();
		f = fopen(fTmp, "w");
		if (i == 'd')
			fprintf(f,
					"[Global]\nText editor = gedit ;\nTerminal emulator = xterm ;\nLocale = en_US.UTF-8 ;\n"
							"[Binds]\nAnother window = a ;\nQuit = q ;\nHidden mode = h ;\nSame folder = s ;\n"
							"Change folder = c ;\nParent folder = u ;\nGo to line = g ;\nInfo = i ;\nExecute command = k ;\n"
							"Search = / ;\n[uBinds]\n\n[Filetypes]\nTypes = ;\n\n");
		else
			setKeys(f, actions);
		clear();
		printw(
				"Config saved to ~/.config/sfm/sfm.conf\nYou can set user-binds and filetypes in.\nPress any key");
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
	if (sTmp[0] == '\0')
		keys[i++] = 9;
	else
		keys[i++] = sTmp[0];
	while (i < ACTIONS) {
		sprintf(fTmp, "binds:%s", actions[i]);
		strcpy(sTmp, iniparser_getstring(ini, fTmp, NULL ));
		keys[i++] = sTmp[0];
	}
	getUserBinds(ubinds, numbOfBinds, ini);
	getFileTypes(ftypes, numbOfFTypes, ini);
	iniparser_freedict(ini);
	endwin();
}

void reSize(sfmWindowStruct *fWind, sfmWindowStruct *sWind, int *mRow,
		int *mCol) {
	int tmX, tmY;

	getmaxyx(stdscr, tmY, tmX);
	if ((*mRow != tmY) || (*mCol != tmX)) {
		if (fWind->win)
			delwin(fWind->win);
		if (sWind->win)
			delwin(sWind->win);
		wclear(stdscr);
		fWind->win = newwin(tmY - 2, tmX / 2, 1, 0);
		sWind->win = newwin(tmY - 2, tmX / 2, 1, tmX / 2);
		*mRow = tmY;
		*mCol = tmX;
	}
}

void saveConf(sfmPart *fPart, sfmPart *sPart) {
	FILE *f;

	sprintf(fTmp, "%s/.config/sfm/tmp", getenv("HOME"));
	f = fopen(fTmp, "w");
	fprintf(f, "[Main]\nlPath = %s ;\nrPath = %s ;\nlHid = %d ;\nrHid = %d ;\n",
			fPart->sFold.path, sPart->sFold.path, fPart->sFold.showHidden,
			sPart->sFold.showHidden);
	fclose(f);
}

void setKeys(FILE *f, const char **actions) {
	int i = 0, ch;

	echo();
	clear();
	printw("Enter name of your text editor: ");
	scanw("%[^\n]", fTmp);
	fprintf(f, "[Global]\nText editor = %s ;\n", fTmp);
	clear();
	printw("Enter name of your terminal emulator\nwith flags to execute: ");
	scanw("%[^\n]", fTmp);
	fprintf(f, "Terminal emulator = %s ;\nLocale = %s ;\n[Binds]\n", fTmp,
			getenv("LANG"));
	noecho();
	while (i < ACTIONS) {
		clear();
		printw("Enter symbol to action \"%s\": ", actions[i]);
		ch = getch();
		fprintf(f, "%s = %c ;\n", actions[i++], ch);
	}
	fprintf(f, "[uBinds]\n\n[Filetypes]\nTypes = ;\n\n");
}

void setTop(sfmWindowStruct *wind, int mRow, int mkSt) {
	if (mkSt)
		wind->currentLine = 0;
	if (wind->currentLine > mRow - 5)
		wind->top = wind->currentLine - (mRow - 5);
	else
		wind->top = 0;
}

void showCBinds(sfmPart *part, int mRow, sfmBind **ubinds, int numbOfBinds) {
	int i = 0, tY, key = 0;

	while (1) {
		tY = 0;
		wclear(part->sWind.win);
		box(part->sWind.win, 0, 0);
		while ((tY < mRow - 4) && (i + tY < numbOfBinds)) {
			mvwprintw(part->sWind.win, tY + 1, 1, "%c = %s",
					ubinds[i + tY]->key, ubinds[i + tY]->cmd);
			++tY;
		}
		move(mRow - 1, 0);
		wrefresh(part->sWind.win);
		key = getch();
		if (key == KEY_UP)
			--i;
		else if (key == KEY_DOWN)
			++i;
		else
			break;
		if ((i < 0) || (i + tY > numbOfBinds))
			i = 0;
	}
}

int slt(const struct dirent *d) {
	return strncmp(d->d_name, ".", 1);
}
