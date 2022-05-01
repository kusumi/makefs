[makefs(8)](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8) for Linux
===

## About

+ A Linux port of FreeBSD makefs(8)

## Changes

+ 2022.05.01 - Sync with [FreeBSD 835ee05f3c754d905099a3500f421dc01fab028f](https://cgit.freebsd.org/src/commit/?id=835ee05f3c754d905099a3500f421dc01fab028f)

## Supported file systems

|file system     |type name       |original implementation                 |
|:---------------|:---------------|:---------------------------------------|
|UFS             |ffs (default)   |FreeBSD makefs(8)                       |
|FAT             |msdos           |FreeBSD makefs(8)                       |
|ISO9660         |cd9660          |FreeBSD makefs(8)                       |

## Build

        $ cd makefs
        $ make

## Usage examples

        $ ./makefs -t ffs /path/to/img /path/to/directory
        $ ./makefs -t ffs -o version=2 /path/to/img /path/to/directory
        $ ./makefs -t msdos -o create_size=4g /path/to/img /path/to/directory
        $ ./makefs -t cd9660 /path/to/img /path/to/directory

## Install (optional)

        $ cd makefs
        $ make && make install

## Uninstall (optional)

        $ cd makefs
        $ make uninstall

## Note

+ Build confirmed on Fedora, Ubuntu, and Cygwin.

    + No \*.so dependencies, only libc required.

+ mtree(5) related options are currently unsupported.

    + *-F* option, *-N* option, and mtree file input will fail with an error message.

+ Due to lack of UFS standard among vendors, use an appropriate *"-o ufstype=..."* mount option to mount UFS on Linux.

    + Use *"-o ufstype=44bsd"* (4.4 BSD) for an image created with *"-o version=1"* or the default.

    + Use *"-o ufstype=ufs2"* (FreeBSD UFS2) for an image created with *"-o version=2"*.

## Resource

+ makefs(8) [https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8](https://www.freebsd.org/cgi/man.cgi?query=makefs&sektion=8)

+ mtree(5) [https://www.freebsd.org/cgi/man.cgi?query=mtree&sektion=5](https://www.freebsd.org/cgi/man.cgi?query=mtree&sektion=5)
