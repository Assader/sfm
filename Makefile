CC=gcc
PNAME=sfm
PFILE=main
LIBS=-lncurses
OBJS=dictionary.o iniparser.o
FLAGS=-O2 -Wall -pedantic -o ${PNAME}
OFLAGS=-O3 -Wall -pedantic -march=native -mtune=native -o ${PNAME}
all: ${PFILE}.c inip
	${CC} ${FLAGS} ${PFILE}.c ${OBJS} ${LIBS}
optimised: ${PFILE}.c inip
	${CC} ${OFLAGS} ${PFILE}.c ${OBJS} ${LIBS}
inip:
	${CC} -c -O2 -Wall -ansi -pedantic ./iniparser/src/dictionary.c
	${CC} -c -O2 -Wall -ansi -pedantic ./iniparser/src/iniparser.c
clean:
	rm -f *.o
	rm -f ${PNAME}
install:
	cp ${PNAME} /usr/bin/
uninstall:
	rm /usr/bin/${PNAME}
remake:
	make clean
	make
