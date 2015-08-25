SUBDIRS = pan tiobench bonnie++ fs_mark iozone iperf ltp lmbench3 nbench-byte

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean ; done

