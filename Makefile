CC=gcc
CFLAGS=-Wall -O3 -std=c99
PREFIX=~/.local

all: bhagchal

install: bhagchal tkchal.py
	cp bhagchal $(PREFIX)/bin
	cp tkchal.py $(PREFIX)/bin/tkchal
	chmod a+x $(PREFIX)/bin/tkchal

bhagchal: bhagchal.c bhagchal.h
