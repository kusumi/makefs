SRCS=	subr_sbuf.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -O2
CFLAGS+=	-I. -I..

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
