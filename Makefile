SUBDIRS = usr.sbin/makefs usr.sbin/makefs/cd9660 usr.sbin/makefs/ffs usr.sbin/makefs/msdos sbin/newfs_msdos sys/kern sys/ufs/ffs contrib/libc-vis contrib/mtree lib/libc/gen lib/libc/string lib/libnetbsd
BINDIRS = usr.sbin/makefs

.PHONY: all clean $(SUBDIRS) $(BINDIRS)

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
usr.sbin/makefs: $(SUBDIRS)
clean:
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
