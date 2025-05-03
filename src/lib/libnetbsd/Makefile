SRCS=	efun.c strsuftoll.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -O2
CFLAGS+=	-I. -I../../sys

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
