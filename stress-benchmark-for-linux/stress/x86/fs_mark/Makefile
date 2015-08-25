#
# Simple makefile for file latency test program
#
# fs_mark.c is a modified version of Larry McVoy's lmbench program.
#
# Modifications include using fsync after wrting to flush to disk and changes to check return
# values from syscalls.
#

INSTALL=/usr/bin/install -c

EXES=fs_mark

prefix=${INSTALLDIR}
eprefix=${prefix}

DIR1= /test/dir1
DIR2= /test/dir2

COBJS= fs_mark.o lib_timing.o
CFLAGS= -O2 -Wall -D_FILE_OFFSET_BITS=64

all: fs_mark 

fs_mark.o: fs_mark.c fs_mark.h

fs_mark: fs_mark.o lib_timing.o
	${CC} -static -o fs_mark fs_mark.o lib_timing.o

install:
	${INSTALL} -s $(EXES) $(eprefix)/bin

clean:
	rm -f ${COBJS} fs_mark fs_log.txt

