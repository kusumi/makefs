SUBDIRS = usr.sbin/makefs usr.sbin/makefs/cd9660 usr.sbin/makefs/ffs usr.sbin/makefs/msdos usr.sbin/makefs/v7fs sbin/fsck sbin/newfs_msdos sbin/newfs_v7fs sys/kern sys/fs/v7fs sys/ufs/ffs contrib/libc-vis contrib/mtree lib/libc/gen lib/libc/string lib/libnetbsd
BINDIRS = usr.sbin/makefs

# HAMMER2 enabled by default
export USE_HAMMER2	?= 1
ifeq ($(USE_HAMMER2), 1)
	SUBDIRS	+= usr.sbin/makefs/hammer2 usr.sbin/makefs/hammer2/zlib sbin/hammer2 sbin/newfs_hammer2 sys/libkern sys/vfs/hammer2/xxhash
endif

# exFAT enabled by default
export USE_EXFAT	?= 1
ifeq ($(USE_EXFAT), 1)
	SUBDIRS	+= gpl/github.com/relan/exfat/libexfat gpl/github.com/relan/exfat/dump gpl/github.com/relan/exfat/fsck gpl/github.com/relan/exfat/label gpl/github.com/relan/exfat/mkfs
endif

.PHONY: all clean $(SUBDIRS) $(BINDIRS)

all: $(SUBDIRS)
	if [ ! -e ./makefs ]; then \
		ln -s usr.sbin/makefs/makefs ./makefs; \
	fi
	if [ ! -e ./exfat ]; then \
		ln -s gpl/github.com/relan/exfat ./exfat; \
	fi
$(SUBDIRS):
	$(MAKE) -C $@
usr.sbin/makefs: $(SUBDIRS)
gpl/github.com/relan/exfat/dump: gpl/github.com/relan/exfat/libexfat
gpl/github.com/relan/exfat/fsck: gpl/github.com/relan/exfat/libexfat
gpl/github.com/relan/exfat/label: gpl/github.com/relan/exfat/libexfat
gpl/github.com/relan/exfat/mkfs: gpl/github.com/relan/exfat/libexfat
clean:
	if [ -e ./makefs ]; then \
		rm ./makefs; \
	fi
	if [ -e ./exfat ]; then \
		rm ./exfat; \
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
