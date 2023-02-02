SRCS=	v7fs_estimate.c v7fs_populate.c

OBJS := $(SRCS:.c=.o)

CC=	gcc
CFLAGS+=	-Wall -g
CFLAGS+=	-DV7FS_EI
# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../sys/fs/v7fs -I../../../sbin/newfs_v7fs

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
