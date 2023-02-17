.PHONY: all test benchmark install clean

prefix = /usr/local
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib
includedir = $(prefix)/include

CC = cc
CFLAGS = -std=c89 -pedantic-errors
INSTALL = install

ALL = libaca.a aca.o

all: $(ALL)

aca.o: aca.c aca.h
	$(CC) $(CFLAGS) -c -o $@ $<

libaca.a: aca.o
	ar -cr $@ $^

run.tmp: aca.o aca.h test.c
	$(CC) $(CFLAGS) -o $@ aca.o test.c

naive/run.tmp:
	(cd naive && make)

test: run.tmp
	./runtest

benchmark: test
	(cd naive; make test)
	./benchmark

install: all
	$(INSTALL) -d $(includedir) $(libdir)
	$(INSTALL) aca.h $(includedir)/
	$(INSTALL) libaca.a $(libdir)/

clean:
	rm -rf $(ALL) *.tmp
	(cd naive; make clean)
