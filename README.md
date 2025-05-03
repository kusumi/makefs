[FreeBSD makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8) for Linux / *BSD
========

## [About](src/usr.sbin/makefs/makefs.8.txt)

FreeBSD makefs(8) + HAMMER2 support + exFAT support

## Supported platforms

Linux, Cygwin, FreeBSD, NetBSD, OpenBSD, DragonFly BSD

## Supported file systems

|file system |*-t* option name |original implementation |
|:-----------|:----------------|:-----------------------|
|UFS         |ffs (default)    |[FreeBSD makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8)|
|FAT         |msdos            |[FreeBSD makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8)|
|ISO9660     |cd9660           |[FreeBSD makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8)|
|HAMMER2     |hammer2          |[DragonFly BSD makefs(8)](https://man.dragonflybsd.org/?command=makefs)|
|exFAT       |exfat            |[https://github.com/relan/exfat](https://github.com/relan/exfat)|

## Build

+ By default all supported file systems above are enabled. Use gmake(1) on *BSD.

        $ cd makefs
        $ make

+ Specify *USE_HAMMER2=0* to disable HAMMER2 support.

        $ cd makefs
        $ make USE_HAMMER2=0

+ Specify *USE_EXFAT=0* to disable exFAT support.

        $ cd makefs
        $ make USE_EXFAT=0

## Notes

+ mtree(5) related options are unsupported. *-F* option, *-N* option, and mtree file input will fail with an error message.

+ *"-t zfs"* option in FreeBSD makefs(8) is unsupported.

## License

+ [src/gpl](src/gpl) and [src/usr.sbin/makefs/exfat_gpl.c](src/usr.sbin/makefs/exfat_gpl.c) are under [GPL v2](src/gpl/github.com/relan/exfat/COPYING).

+ [HAMMER2](src/usr.sbin/makefs/hammer2) and other files derived from DragonFly BSD are under [BSDL](src/usr.sbin/makefs/hammer2/COPYRIGHT).

+ All other files are under [BSDL](COPYRIGHT).
