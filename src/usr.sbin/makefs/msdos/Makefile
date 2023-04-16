SRCS=	msdosfs_conv.c msdosfs_denode.c msdosfs_fat.c msdosfs_lookup.c msdosfs_vfsops.c msdosfs_vnops.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-DMAKEFS
# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../sbin/newfs_msdos -I../../../lib/libnetbsd -I../../../lib/libutil

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
