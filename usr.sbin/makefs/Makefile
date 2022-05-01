PROG=	makefs
SRCS=	makefs.c mtree.c walk.c cd9660.c ffs.c msdos.c

OBJS := $(SRCS:.c=.o)

CC=	gcc
CFLAGS+=	-Wall -g
CFLAGS+=	-DMAKEFS
CFLAGS+=	-I. -I../../sbin/newfs_msdos -I../../sys -I../../sys/fs/cd9660 -I../../contrib/libc-vis -I../../contrib/mtree -I../../lib/libnetbsd -I../../lib/libutil

.PHONY: all clean

all: $(PROG)
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) ./cd9660/cd9660_archimedes.o ./cd9660/cd9660_conversion.o ./cd9660/cd9660_debug.o ./cd9660/cd9660_eltorito.o ./cd9660/cd9660_strings.o ./cd9660/cd9660_write.o ./cd9660/iso9660_rrip.o ./ffs/buf.o ./ffs/ffs_alloc.o ./ffs/ffs_balloc.o ./ffs/ffs_bswap.o ./ffs/ffs_subr.o ./ffs/mkfs.o ./ffs/ufs_bmap.o ./msdos/msdosfs_conv.o ./msdos/msdosfs_denode.o ./msdos/msdosfs_fat.o ./msdos/msdosfs_lookup.o ./msdos/msdosfs_vfsops.o ./msdos/msdosfs_vnops.o ../../sbin/newfs_msdos/mkfs_msdos.o ../../sys/kern/subr_sbuf.o ../../sys/ufs/ffs/ffs_tables.o ../../contrib/libc-vis/unvis.o ../../contrib/mtree/misc.o ../../lib/libc/gen/err.o ../../lib/libc/gen/getprogname.o ../../lib/libc/gen/setprogname.o ../../lib/libc/string/strlcat.o ../../lib/libc/string/strlcpy.o ../../lib/libnetbsd/efun.o ../../lib/libnetbsd/strsuftoll.o
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
