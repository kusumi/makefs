PROG=	exfatlabel
SRCS=	main.c

OBJS := $(SRCS:.c=.o)

CC=	gcc
CFLAGS+=	-Wall -g
CFLAGS+=	-I. -I../libexfat

.PHONY: all clean

all: $(PROG)
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) ../libexfat/cluster.o ../libexfat/io.o ../libexfat/log.o ../libexfat/lookup.o ../libexfat/mount.o ../libexfat/node.o ../libexfat/repair.o ../libexfat/time.o ../libexfat/utf.o ../libexfat/utils.o
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o ./$(PROG)
