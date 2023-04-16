SRCS=	subr_sbuf.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-I. -I..

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
