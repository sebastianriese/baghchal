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

install: bhagchal tkchal.py
	chmod $(INSTALL_MODE) bhagchal
	cp -f --preserve=mode bhagchal $(PREFIX)/bin
	cp tkchal.py tkchal
	chmod $(INSTALL_MODE) tkchal
	cp -f --preserve=mode tkchal $(PREFIX)/bin

bhagchal: bhagchal.c bhagchal.h movedb.h movedb.o

movedb_inspect: movedb_inspect.c movedb.h movedb.o

movedb.o : movedb.h movedb.c
