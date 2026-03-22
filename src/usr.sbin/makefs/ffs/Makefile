SRCS:=	buf.c ffs_alloc.c ffs_balloc.c ffs_bswap.c ffs_subr.c mkfs.c ufs_bmap.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../lib/libnetbsd

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
