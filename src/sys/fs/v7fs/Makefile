SRCS=	v7fs_datablock.c v7fs_dirent.c v7fs_endian.c v7fs_file.c v7fs_file_util.c v7fs_inode.c v7fs_io.c v7fs_io_user.c v7fs_superblock.c

OBJS := $(SRCS:.c=.o)

CFLAGS+=	-Wall -g
CFLAGS+=	-Wno-address-of-packed-member
CFLAGS+=	-DV7FS_EI
CFLAGS+=	-I. -I../../

.PHONY: all clean

all: $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f ./*.o
