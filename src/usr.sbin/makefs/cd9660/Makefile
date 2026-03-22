SRCS:=	cd9660_conversion.c cd9660_debug.c cd9660_eltorito.c cd9660_strings.c cd9660_write.c iso9660_rrip.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

# not -I.
CFLAGS+=	-I.. -I../../../sys -I../../../sys/fs/cd9660 -I../../../lib/libnetbsd

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS)
-include $(DEPS)
