CC=gcc
CFLAGS=-Wall -O3 -std=c99
LIBS=-lm
LDFLAGS=$(LIBS)
PREFIX=~/.local
INSTALL_MODE=0555

all: baghchal

clean:
	rm baghchal
	rm movedb.o
	rm movedb_inspect
	rm tkchal

install: baghchal tkchal.py
	chmod $(INSTALL_MODE) baghchal
	cp -f --preserve=mode baghchal $(PREFIX)/bin
	cp tkchal.py tkchal
	chmod $(INSTALL_MODE) tkchal
	cp -f --preserve=mode tkchal $(PREFIX)/bin

baghchal: baghchal.c baghchal.h movedb.h movedb.o

movedb_inspect: movedb_inspect.c movedb.h movedb.o

movedb.o : movedb.h movedb.c
