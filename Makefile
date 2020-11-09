CC=c99
nmps: nmps.c
	${CC} -o $@ $< -pedantic -Wall -Wextra
