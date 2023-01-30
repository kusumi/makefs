[makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8) for Linux
===

## About

+ A Linux port of FreeBSD makefs(8) + HAMMER2 support + exFAT support

+ Also see [makefs(8)](usr.sbin/makefs/makefs.8.txt)

## Supported file systems

|file system                |*-t* option name |original implementation        |
|:--------------------------|:----------------|:------------------------------|
|UFS                        |ffs (default)    |FreeBSD makefs(8)              |
|FAT                        |msdos            |FreeBSD makefs(8)              |
|ISO9660                    |cd9660           |FreeBSD makefs(8)              |
|Version 7 Unix file system |v7fs             |NetBSD makefs(8)               |
|HAMMER2                    |hammer2          |DragonFly BSD makefs(8)        |
|exFAT                      |exfat            |https://github.com/relan/exfat |

## Build

+ By default all supported file systems above are enabled.

        $ cd makefs
        $ make

+ Specify *USE_EXFAT=0* to disable exFAT support.

        $ cd makefs
        $ make USE_EXFAT=0

## Usage examples

+ 4.4BSD FFS

        $ ./makefs -t ffs /path/to/img /path/to/directory

+ FreeBSD UFS2

        $ ./makefs -t ffs -o version=2 /path/to/img /path/to/directory

+ ISO9660

        $ ./makefs -t cd9660 /path/to/img /path/to/directory

+ HAMMER2

        $ ./makefs -t hammer2 /path/to/img /path/to/directory

+ FAT12/16/32 (file size required)

        $ ./makefs -t msdos -s 1g /path/to/img /path/to/directory

+ exFAT (file size required)

        $ ./makefs -t exfat -s 1g /path/to/img /path/to/directory

## Install (optional)

        $ cd makefs
        $ make && make install

## Uninstall (optional)

        $ cd makefs
        $ make uninstall

## Note

+ Build confirmed on Fedora, Ubuntu, and Cygwin.

    + No \*.so dependencies, only libc required.

+ Due to lack of UFS standard among vendors, use an appropriate *"-o ufstype=..."* mount option to mount UFS on Linux.

    + Use *"-o ufstype=44bsd"* (4.4BSD) for an image created with *"-o version=1"* or the default.

    + Use *"-o ufstype=ufs2"* (FreeBSD UFS2) for an image created with *"-o version=2"*.

+ ZFS support which exists in recent FreeBSD makefs(8) is unsupported.

## Bug

+ mtree(5) related options are currently unsupported.

    + *-F* option, *-N* option, and mtree file input will fail with an error message.

+ v7fs support compiles, but *"-t v7fs"* option not working.

    + *"-t v7fs"* option does not work on NetBSD makefs(8) as well.

## License

+ Files under *gpl/* directory and *usr.sbin/makefs/exfat.c* are under GPL v2.

    + See *gpl/github.com/relan/exfat/COPYING*.

+ HAMMER2 and other files derived from DragonFly BSD are under BSDL.

    + See *COPYRIGHT.dragonfly*.

+ All other files are under BSDL.

    + See *COPYRIGHT*.

## Resource

+ FreeBSD makefs(8) [https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8)

+ FreeBSD mtree(5) [https://www.freebsd.org/cgi/man.cgi?query=mtree&sektion=5](https://www.freebsd.org/cgi/man.cgi?query=mtree&sektion=5)

+ NetBSD makefs(8) [https://man.netbsd.org/makefs.8](https://man.netbsd.org/makefs.8)

+ DragonFly BSD makefs(8) [https://man.dragonflybsd.org/?command=makefs](https://man.dragonflybsd.org/?command=makefs)

+ "Free exFAT file system implementation" [https://github.com/relan/exfat](https://github.com/relan/exfat)
