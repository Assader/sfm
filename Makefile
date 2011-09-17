PNAME=sfm
PVER=2c
FLAGS=-Wall -O2 -lncurses
${PNAME}: ${PNAME}${PVER}.c
	gcc ${FLAGS} -o ${PNAME} ${PNAME}${PVER}.c
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
