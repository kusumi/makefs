SRCS:=	msdosfs_conv.c msdosfs_denode.c msdosfs_fat.c msdosfs_lookup.c msdosfs_vfsops.c msdosfs_vnops.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

CFLAGS+=	-DMAKEFS
# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../sbin/newfs_msdos -I../../../lib/libnetbsd

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
