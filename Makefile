CC=c99
nmps: nmps.c config.h
	${CC} -o $@ $< -pedantic -Wall -Wextra
