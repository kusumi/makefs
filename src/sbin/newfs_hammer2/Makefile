SRCS=	mkfs_hammer2.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-I. -I../../sys -I../hammer2

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
