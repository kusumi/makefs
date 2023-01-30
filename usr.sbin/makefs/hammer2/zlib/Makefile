SRCS=	hammer2_zlib_adler32.c hammer2_zlib_deflate.c hammer2_zlib_inffast.c hammer2_zlib_inflate.c hammer2_zlib_inftrees.c hammer2_zlib_trees.c hammer2_zlib_zutil.c

OBJS := $(SRCS:.c=.o)

CC=	gcc
CFLAGS+=	-Wall -g
CFLAGS+=	-I. -I../../../../sys -I../.. -I../../../../sbin/hammer2 -I../../../../sbin/newfs_hammer2 -I../../../../lib/libnetbsd -I../../../../lib/libutil

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
