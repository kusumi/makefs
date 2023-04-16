SRCS=	main.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-DV7FS_EI
CFLAGS+=	-I. -I../../sys -I../../sys/fs/v7fs -I../../sbin/fsck

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
