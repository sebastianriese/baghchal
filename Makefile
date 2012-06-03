CC=gcc
CFLAGS=-Wall -O3 -std=c99
PREFIX=~/.local
INSTALL_MODE=0555

all: baghchal

clean:
	rm baghchal

install: baghchal tkchal.py
	cp -f baghchal $(PREFIX)/bin
	cp -f tkchal.py $(PREFIX)/bin/tkchal
	chmod $(INSTALL_MODE) $(PREFIX)/bin/baghchal
	chmod $(INSTALL_MODE) $(PREFIX)/bin/tkchal

baghchal: baghchal.c baghchal.h
