CC=gcc
CFLAGS=-Wall -O3 -std=c99
LIBS=-lm
LDFLAGS=$(LIBS)

OBJECTS=movedb.o normalize.o

PREFIX=~/.local
INSTALL_MODE=0555
INSTALL_BIN=tkchal baghchal

all: baghchal

clean:
	@# TODO: check for existence!
	rm $(OBJECTS)
	rm baghchal
	rm movedb_inspect
	rm tkchal

install: $(INSTALL_BIN)
	install -t $(PREFIX)/bin -m $(INSTALL_MODE) $(INSTALL_BIN)

tkchal: tkchal.py
	cp -f tkchal.py tkchal

baghchal: baghchal.c baghchal.h $(OBJECTS)

movedb_inspect: movedb_inspect.c $(OBJECTS)

movedb.o : movedb.h movedb.c normalize.o

normalize.o : normalize.h normalize.c
