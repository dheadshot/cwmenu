CC = gcc
XFTFLAGS = -DHAVE_XFT

cwmenu: main.o xfuncs.o
	${CC} ${XFTFLAGS} main.o xfuncs.o -lX11 -lXft -o cwmenu
	chmod +x cwmenu

main.o: main.c
	${CC} ${XFTFLAGS} -c main.c

xfuncs.o: xfuncs.c
	${CC} ${XFTFLAGS} -c xfuncs.c

sfuncs.o: sfuncs.c
	${CC} -c sfuncs.c
