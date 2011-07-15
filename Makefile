PNAME=sfm
PVER=2b
FLAGS=-g -Wall -O2 -lncurses
${PNAME}: ${PNAME}${PVER}.o
	gcc ${FLAGS} -o ${PNAME} ${PNAME}${PVER}.o
${PNAME}${PVER}.o: ${PNAME}${PVER}.c
	gcc -c ${PNAME}${PVER}.c
clean:
	rm -f *.o
	rm -f ${PNAME}
install:
	cp ${PNAME} /usr/bin
uninstall:
	rm /usr/bin/${PNAME}
remake:
	make clean
	make
