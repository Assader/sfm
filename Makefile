PNAME=sfm
PFILE=main
FLAGS=-Wall -pedantic -O2 -lncurses -o ${PNAME}
OFLAGS=-Wall -pedantic -O3 -g -lncurses -march=native -mtune=native -o ${PNAME}
${PNAME}: ${PFILE}.c
	gcc ${FLAGS} ${PFILE}.c
optimised: ${PFILE}.c
	gcc ${OFLAGS} main.c 
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
