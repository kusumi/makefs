SUBDIRS = usr.sbin/makefs usr.sbin/makefs/cd9660 usr.sbin/makefs/ffs usr.sbin/makefs/msdos usr.sbin/makefs/v7fs sbin/fsck sbin/newfs_msdos sbin/newfs_v7fs sys/kern sys/fs/v7fs sys/ufs/ffs contrib/libc-vis contrib/mtree lib/libc/gen lib/libc/string lib/libnetbsd
BINDIRS = usr.sbin/makefs

.PHONY: all clean $(SUBDIRS) $(BINDIRS)

all: $(SUBDIRS)
	if [ ! -e ./makefs ]; then \
		ln -s usr.sbin/makefs/makefs ./makefs; \
	fi
$(SUBDIRS):
	$(MAKE) -C $@
usr.sbin/makefs: $(SUBDIRS)
clean:
	if [ -e ./makefs ]; then \
		rm ./makefs; \
	fi
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
install:
	for dir in $(BINDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
uninstall:
	for dir in $(BINDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
