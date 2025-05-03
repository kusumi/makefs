SRCS=	mkfs_msdos.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -O2
CFLAGS+=	-DMAKEFS
CFLAGS+=	-I. -I../../sys

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
