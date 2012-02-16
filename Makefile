CC=gcc
CFLAGS=-Wall -O3 -std=c99
LIBS=-lm
LDFLAGS=$(LIBS)
PREFIX=~/.local

all: bhagchal

install: bhagchal tkchal.py
	chmod a-w bhagchal
	cp -p bhagchal $(PREFIX)/bin
	cp tkchal.py tkchal
	chmod a-w tkchal
	chmod a+rx tkchal
	cp -p tkchal $(PREFIX)/bin
	chmod a+x $(PREFIX)/bin/tkchal

bhagchal: bhagchal.c bhagchal.h movedb.h movedb.o

movedb_inspect: movedb_inspect.c movedb.h movedb.o

movedb.o : movedb.h movedb.c
