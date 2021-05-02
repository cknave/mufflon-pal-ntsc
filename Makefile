CC=gcc

COPT=-lm -O5 -ffast-math $(CFLAGS) -Wall -W
#COPT=-lm -g3

.PHONY: clean

all : mufflon

mufflon: mufflon.c mufflon.h
	$(CC) -o $@ $(COPT) $<

clean:
	rm -f mufflon
	rm -f *~

tar:
	- tar -zcvf mufflon.tar.gz mufflon.h mufflon.c Makefile
