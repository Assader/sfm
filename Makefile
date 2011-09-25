PNAME=sfm
PFILE=main
FLAGS=-Wall -O2 -lncurses -o ${PNAME}
${PNAME}: ${PFILE}.c
	gcc ${FLAGS} ${PFILE}.c
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
