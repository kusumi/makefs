SRCS:=	cluster.c io.c log.c lookup.c mount.c node.c repair.c time.c utf.c utils.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

CFLAGS+=	-I.

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
