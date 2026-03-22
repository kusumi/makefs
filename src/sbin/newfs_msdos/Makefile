SRCS:=	mkfs_msdos.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

CFLAGS+=	-DMAKEFS
CFLAGS+=	-I. -I../../sys

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
