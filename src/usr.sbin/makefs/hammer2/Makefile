SRCS=	hammer2_admin.c hammer2_buf.c hammer2_bulkfree.c hammer2_chain.c hammer2_cluster.c hammer2_flush.c hammer2_freemap.c hammer2_inode.c hammer2_io.c hammer2_ioctl.c hammer2_lz4.c hammer2_ondisk.c hammer2_strategy.c hammer2_subr.c hammer2_vfsops.c hammer2_vnops.c hammer2_xops.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../sbin/hammer2 -I../../../sbin/newfs_hammer2 -I../../../lib/libnetbsd -I../../../lib/libutil

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
