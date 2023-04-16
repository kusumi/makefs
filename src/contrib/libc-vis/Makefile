SRCS=	unvis.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-I. -I../../sys

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
