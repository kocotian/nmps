CC=cc
PREFIX=/usr/local
nmps: nmps.c config.h getch.h
	${CC} -o $@ $< -pedantic -Wall -Wextra -std=c99

install: nmps
	install -Dm 755 nmps ${PREFIX}/bin

clean: nmps
	rm -f nmps
