
#   Unix make file for random test program

CC = gcc
CFLAGS = -g -Wall
#CFLAGS = -O

ent:	ent.o iso8859.o randtest.o chisq.o
	$(CC) $(CFLAGS) ent.o iso8859.o randtest.o chisq.o -o ent -lm

ent.c:	iso8859.h randtest.h

DISTFILES = Makefile ent.c ent.exe ent.html \
	    entitle.gif entest.bat entestw32.bat entest.mas \
	    iso8859.c iso8859.h randtest.c randtest.h \
	    chisq.c \
	    ent.sln ent.vcproj

release:
	rm -f random.zip
	zip random.zip $(DISTFILES)

TESTFILE = entitle.gif

check:	ent
	sh entest.bat
	diff entest.mas entest.bak

lint:
	lint *.c

clean:
	rm -f *.o *.bak ent ent.zip
