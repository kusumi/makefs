PROG=	makefs
SRCS=	makefs.c mtree.c walk.c cd9660.c ffs.c msdos.c v7fs.c
ifeq ($(USE_EXFAT), 1)
	SRCS	+= exfat.c
endif

OBJS := $(SRCS:.c=.o)

CC=	gcc
CFLAGS+=	-Wall -g
CFLAGS+=	-DMAKEFS -DV7FS_EI
ifeq ($(USE_EXFAT), 1)
	CFLAGS	+= -DMAKEFS_EXFAT
endif
# -I../../gpl/github.com/relan/exfat/libexfat is for gpl/github.com/relan/exfat/mkfs/mkexfat.h
CFLAGS+=	-I. -I../../sys -I../../sys/fs/cd9660 -I../../sys/fs/v7fs -I../../sbin/fsck -I../../sbin/newfs_msdos -I../../sbin/newfs_v7fs -I../../contrib/libc-vis -I../../contrib/mtree -I../../lib/libnetbsd -I../../lib/libutil -I../../gpl/github.com/relan -I../../gpl/github.com/relan/exfat/libexfat

EXFAT_LIBEXFAT_OBJS=
EXFAT_MKFS_OBJS=
ifeq ($(USE_EXFAT), 1)
	EXFAT_LIBEXFAT_DIR=	../../gpl/github.com/relan/exfat/libexfat
	EXFAT_LIBEXFAT_OBJS+=	$(EXFAT_LIBEXFAT_DIR)/cluster.o $(EXFAT_LIBEXFAT_DIR)/io.o $(EXFAT_LIBEXFAT_DIR)/log.o $(EXFAT_LIBEXFAT_DIR)/lookup.o $(EXFAT_LIBEXFAT_DIR)/mount.o $(EXFAT_LIBEXFAT_DIR)/node.o $(EXFAT_LIBEXFAT_DIR)/repair.o $(EXFAT_LIBEXFAT_DIR)/time.o $(EXFAT_LIBEXFAT_DIR)/utf.o $(EXFAT_LIBEXFAT_DIR)/utils.o
	EXFAT_MKFS_DIR=	../../gpl/github.com/relan/exfat/mkfs
	EXFAT_MKFS_OBJS+=	$(EXFAT_MKFS_DIR)/cbm.o $(EXFAT_MKFS_DIR)/fat.o $(EXFAT_MKFS_DIR)/mkexfat.o $(EXFAT_MKFS_DIR)/rootdir.o $(EXFAT_MKFS_DIR)/uct.o $(EXFAT_MKFS_DIR)/uctc.o $(EXFAT_MKFS_DIR)/vbr.o
endif

.PHONY: all clean

all: $(PROG)
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) ./cd9660/cd9660_archimedes.o ./cd9660/cd9660_conversion.o ./cd9660/cd9660_debug.o ./cd9660/cd9660_eltorito.o ./cd9660/cd9660_strings.o ./cd9660/cd9660_write.o ./cd9660/iso9660_rrip.o ./ffs/buf.o ./ffs/ffs_alloc.o ./ffs/ffs_balloc.o ./ffs/ffs_bswap.o ./ffs/ffs_subr.o ./ffs/mkfs.o ./ffs/ufs_bmap.o ./msdos/msdosfs_conv.o ./msdos/msdosfs_denode.o ./msdos/msdosfs_fat.o ./msdos/msdosfs_lookup.o ./msdos/msdosfs_vfsops.o ./msdos/msdosfs_vnops.o ./v7fs/v7fs_estimate.o ./v7fs/v7fs_populate.o ../../sbin/fsck/progress.o ../../sbin/newfs_msdos/mkfs_msdos.o ../../sbin/newfs_v7fs/main.o ../../sys/kern/subr_sbuf.o ../../sys/ufs/ffs/ffs_tables.o ../../sys/fs/v7fs/v7fs_datablock.o ../../sys/fs/v7fs/v7fs_dirent.o ../../sys/fs/v7fs/v7fs_endian.o ../../sys/fs/v7fs/v7fs_file.o ../../sys/fs/v7fs/v7fs_file_util.o ../../sys/fs/v7fs/v7fs_inode.o ../../sys/fs/v7fs/v7fs_io.o ../../sys/fs/v7fs/v7fs_io_user.o ../../sys/fs/v7fs/v7fs_superblock.o ../../contrib/libc-vis/unvis.o ../../contrib/mtree/misc.o ../../lib/libc/gen/err.o ../../lib/libc/gen/getprogname.o ../../lib/libc/gen/setprogname.o ../../lib/libc/string/strlcat.o ../../lib/libc/string/strlcpy.o ../../lib/libnetbsd/efun.o ../../lib/libnetbsd/strsuftoll.o $(EXFAT_LIBEXFAT_OBJS) $(EXFAT_MKFS_OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o ./$(PROG)
install:
	install -m 755 ./$(PROG) /usr/local/sbin/ || exit 1
	cat ./$(PROG).8 | gzip -9 -n > ./$(PROG).8.gz || exit 1
	install -m 644 ./$(PROG).8.gz /usr/local/share/man/man8/ || exit 1
	rm ./$(PROG).8.gz || exit 1
uninstall:
	rm /usr/local/sbin/$(PROG) || exit 1
	rm /usr/local/share/man/man8/$(PROG).8.gz || exit 1
