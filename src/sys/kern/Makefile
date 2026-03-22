SRCS:=	subr_sbuf.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

CFLAGS+=	-I. -I..

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
