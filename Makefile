CC = gcc

cwmenu: main.o xfuncs.o
	${CC} main.o xfuncs.o -o cwmenu
	chmod +x cwmenu

main.o: main.c
	${CC} -c main.c

xfuncs.o: xfuncs.c
	${CC} -c xfuncs.c

