CROSS_PREFIX=armv5tel-uclibc-linux-gnueabi-

CC=gcc
CFLAGS=-std=c99 -D_XOPEN_SOURCE=600
LDFLAGS=-static

all: mlocker mtester

mlocker: mlocker.c
	$(CROSS_PREFIX)$(CC) $< -o mlocker $(CFLAGS) $(LDFLAGS)

mtester: mtester.c
	$(CROSS_PREFIX)$(CC) $< -o mtester $(CFLAGS) $(LDFLAGS)

push: mlocker mtester
	/opt/packages/android/platform-tools/adb push mlocker /data/jsnell/mlocker 
	/opt/packages/android/platform-tools/adb push mtester /data/jsnell/mtester

clean:
	rm -f mlocker mtester
