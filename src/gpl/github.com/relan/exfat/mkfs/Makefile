PROG:=	mkexfatfs
SRCS:=	cbm.c fat.c main.c mkexfat.c rootdir.c uct.c uctc.c vbr.c

OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:.o=.d)

CFLAGS+=	-I. -I../libexfat

.PHONY: all clean

all: $(PROG)
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) ../libexfat/cluster.o ../libexfat/io.o ../libexfat/log.o ../libexfat/lookup.o ../libexfat/node.o ../libexfat/repair.o ../libexfat/time.o ../libexfat/utf.o ../libexfat/utils.o
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) $(DEPS) $(PROG)
-include $(DEPS)
