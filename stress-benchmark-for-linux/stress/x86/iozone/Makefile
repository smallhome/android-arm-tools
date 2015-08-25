#
# Version $Revision: 1.143 $
#
# The makefile for building all versions of iozone for all supported
# platforms
#
# Supports:	hpux, hpux_no_ansi, hpux-10.1, hpux_no_ansi-10.1,
#		sppux, sppux-10.1, ghpux, sppux, 
#		convex, FreeBSD, OpenBSD, OSFV3, OSFV4, OSFV5, SCO
#		SCO_Unixware_gcc,NetBSD,TRU64, Mac OS X

CC	= cc
C89	= c89
GCC	= gcc
CCS	= /usr/ccs/bin/cc
NACC	= /opt/ansic/bin/cc
CFLAGS	=
S10GCCFLAGS    = -m64
S10CCFLAGS     = -m64
FLAG64BIT      = -m64

# If your Linux kernel supports preadv and pwritev system calls 
# and you want iozone to use them, add -DHAVE_PREADV -DHAVE_PWRITEV
# to CFLAGS

INSTALL=/usr/bin/install -c

EXES=iozone fileop pit_server

prefix=${INSTALLDIR}
eprefix=${prefix}

#
# GNU 'C' compiler Linux build with threads, largefiles, async I/O 
#
all:	iozone_linux.o libasync.o libbif.o fileop_linux.o pit_server.o
	$(CC)  -O3 $(LDFLAGS) iozone_linux.o libasync.o libbif.o -lpthread \
		-lrt -o iozone
	$(CC)  -O3 -Dlinux fileop_linux.o -o fileop
	$(CC)  -O3 -Dlinux pit_server.o -o pit_server

install:
	${INSTALL} -s $(EXES) $(eprefix)/bin

clean:
	rm -f *.o iozone fileop pit_server

pit_server.o:	pit_server.c
	@echo ""
	@echo "Building the pit_server"
	@echo ""
	$(CC) -c  $(CFLAGS) pit_server.c  -o pit_server.o 

iozone_linux.o:	iozone.c libbif.c libasync.c
	@echo ""
	@echo "Building iozone for Linux"
	@echo ""
	$(CC) -Wall -c -O3 -Dunix -DHAVE_ANSIC_C -DASYNC_IO -DHAVE_PREAD \
		-DSHARED_MEM -Dlinux -D_LARGEFILE64_SOURCE $(CFLAGS) iozone.c \
		-DNAME='"linux"' -o iozone_linux.o
	$(CC) -Wall -c -O3 -Dunix -DHAVE_ANSIC_C -DASYNC_IO -D_LARGEFILE64_SOURCE \
		-DSHARED_MEM -Dlinux $(CFLAGS) libbif.c -o libbif.o
	$(CC) -Wall -c -O3 -Dunix -Dlinux -DHAVE_ANSIC_C -DASYNC_IO \
		-D_LARGEFILE64_SOURCE $(CFLAGS) libasync.c  -o libasync.o 

fileop_linux.o:	fileop.c
	@echo ""
	@echo "Building fileop for Linux"
	@echo ""
	$(CC) -Wall -c -O3 $(CFLAGS) fileop.c -o fileop_linux.o


