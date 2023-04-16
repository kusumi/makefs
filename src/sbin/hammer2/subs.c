/*
 * Copyright (c) 2011-2012 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@dragonflybsd.org>
 * by Venkatesh Srinivas <vsrinivas@dragonflybsd.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/compat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
//#include <sys/diskslice.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#if defined __linux__ || defined __CYGWIN__
#include <uuid/uuid.h>
#else
#include <uuid.h>
#endif

#ifdef __CYGWIN__
#include <cygwin/fs.h> /* BLKGETSIZE, BLKSSZGET */
#endif

#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/disk.h>
#endif

#ifdef __OpenBSD__
#include <sys/dkio.h>
#include <sys/disklabel.h>
#endif

#ifdef __DragonFly__
#include <sys/diskslice.h>
#endif

#include <vfs/hammer2/hammer2_disk.h>
#include <vfs/hammer2/hammer2_ioctl.h>

#include "hammer2_subs.h"

const char *
hammer2_uuid_to_str(const hammer2_uuid_t *uuid, char **strp)
{
	if (*strp) {
		free(*strp);
		*strp = NULL;
	}
	hammer2_uuid_to_string(uuid, strp);
	return (*strp);
}

const char *
sizetostr(hammer2_off_t size)
{
	static char buf[32];

	if (size < 1024 / 2) {
		snprintf(buf, sizeof(buf), "%6.2fB", (double)size);
	} else if (size < 1024 * 1024 / 2) {
		snprintf(buf, sizeof(buf), "%6.2fKB",
			(double)size / 1024);
	} else if (size < 1024 * 1024 * 1024LL / 2) {
		snprintf(buf, sizeof(buf), "%6.2fMB",
			(double)size / (1024 * 1024));
	} else if (size < 1024 * 1024 * 1024LL * 1024LL / 2) {
		snprintf(buf, sizeof(buf), "%6.2fGB",
			(double)size / (1024 * 1024 * 1024LL));
	} else {
		snprintf(buf, sizeof(buf), "%6.2fTB",
			(double)size / (1024 * 1024 * 1024LL * 1024LL));
	}
	return(buf);
}

hammer2_off_t
check_volume(int fd)
{
	struct stat st;
	hammer2_off_t size;

	/*
	 * Get basic information about the volume
	 */
#if defined __linux__ || defined __CYGWIN__
	if (ioctl(fd, BLKGETSIZE, &size) < 0) {
		/*
		 * Allow the formatting of regular files as HAMMER2 volumes
		 */
		if (fstat(fd, &st) < 0)
			err(1, "Unable to stat fd %d", fd);
		if (!S_ISREG(st.st_mode))
			errx(1, "Unsupported file type for fd %d", fd);
		size = st.st_size;
	} else {
		int sector_size;
		if (ioctl(fd, BLKSSZGET, &sector_size) < 0) {
			errx(1, "Failed to get media sector size");
			/* not reached */
		}
		if (sector_size > HAMMER2_PBUFSIZE ||
		    HAMMER2_PBUFSIZE % sector_size) {
			errx(1, "A media sector size of %d is not supported",
			     sector_size);
		}
		size <<= 9;
	}
#elif defined __FreeBSD__ || defined __NetBSD__
	if (ioctl(fd, DIOCGMEDIASIZE, &size) < 0) {
		/*
		 * Allow the formatting of regular files as HAMMER2 volumes
		 */
		if (fstat(fd, &st) < 0)
			err(1, "Unable to stat fd %d", fd);
		if (!S_ISREG(st.st_mode))
			errx(1, "Unsupported file type for fd %d", fd);
		size = st.st_size;
	} else {
		/*
		 * When formatting a block device as a HAMMER2 volume the
		 * sector size must be compatible.  HAMMER2 uses 64K
		 * filesystem buffers but logical buffers for direct I/O
		 * can be as small as HAMMER2_LOGSIZE (16KB).
		 */
		int media_blksize;
		if (!ioctl(fd, DIOCGSECTORSIZE, &media_blksize) &&
		    (media_blksize > HAMMER2_PBUFSIZE ||
		    HAMMER2_PBUFSIZE % media_blksize)) {
			errx(1, "A media sector size of %d is not supported",
			     media_blksize);
		}
	}
#elif defined __OpenBSD__
	struct disklabel dl;
	if (ioctl(fd, DIOCGDINFO, &dl) < 0) {
		/*
		 * Allow the formatting of regular files as HAMMER2 volumes
		 */
		if (fstat(fd, &st) < 0)
			err(1, "Unable to stat fd %d", fd);
		if (!S_ISREG(st.st_mode))
			errx(1, "Unsupported file type for fd %d", fd);
		size = st.st_size;
	} else {
		/*
		 * When formatting a block device as a HAMMER2 volume the
		 * sector size must be compatible.  HAMMER2 uses 64K
		 * filesystem buffers but logical buffers for direct I/O
		 * can be as small as HAMMER2_LOGSIZE (16KB).
		 */
		int media_blksize = dl.d_secsize;
		size = (hammer2_off_t)dl.d_secperunit * media_blksize;
		if (media_blksize > HAMMER2_PBUFSIZE ||
		    HAMMER2_PBUFSIZE % media_blksize) {
			errx(1, "A media sector size of %d is not supported",
			     media_blksize);
		}
	}
#elif defined __DragonFly__
	struct partinfo pinfo;
	if (ioctl(fd, DIOCGPART, &pinfo) < 0) {
		/*
		 * Allow the formatting of regular files as HAMMER2 volumes
		 */
		if (fstat(fd, &st) < 0)
			err(1, "Unable to stat fd %d", fd);
		if (!S_ISREG(st.st_mode))
			errx(1, "Unsupported file type for fd %d", fd);
		size = st.st_size;
	} else {
		/*
		 * When formatting a block device as a HAMMER2 volume the
		 * sector size must be compatible.  HAMMER2 uses 64K
		 * filesystem buffers but logical buffers for direct I/O
		 * can be as small as HAMMER2_LOGSIZE (16KB).
		 */
		if (pinfo.reserved_blocks) {
			errx(1, "HAMMER2 cannot be placed in a partition "
			    "which overlaps the disklabel or MBR");
		}
		if (pinfo.media_blksize > HAMMER2_PBUFSIZE ||
		    HAMMER2_PBUFSIZE % pinfo.media_blksize) {
			errx(1, "A media sector size of %d is not supported",
			     pinfo.media_blksize);
		}
		size = pinfo.media_size;
	}
#else
#error "Unsupported platform"
#endif
	return(size);
}

/*
 * Borrow HAMMER1's directory hash algorithm #1 with a few modifications.
 * The filename is split into fields which are hashed separately and then
 * added together.
 *
 * Differences include: bit 63 must be set to 1 for HAMMER2 (HAMMER1 sets
 * it to 0), this is because bit63=0 is used for hidden hardlinked inodes.
 * (This means we do not need to do a 0-check/or-with-0x100000000 either).
 *
 * Also, the iscsi crc code is used instead of the old crc32 code.
 */
hammer2_key_t
dirhash(const char *aname, size_t len)
{
	uint32_t crcx;
	uint64_t key;
	size_t i;
	size_t j;

	key = 0;

	/*
	 * m32
	 */
	crcx = 0;
	for (i = j = 0; i < len; ++i) {
		if (aname[i] == '.' ||
		    aname[i] == '-' ||
		    aname[i] == '_' ||
		    aname[i] == '~') {
			if (i != j)
				crcx += hammer2_icrc32(aname + j, i - j);
			j = i + 1;
		}
	}
	if (i != j)
		crcx += hammer2_icrc32(aname + j, i - j);

	/*
	 * The directory hash utilizes the top 32 bits of the 64-bit key.
	 * Bit 63 must be set to 1.
	 */
	crcx |= 0x80000000U;
	key |= (uint64_t)crcx << 32;

	/*
	 * l16 - crc of entire filename
	 *
	 * This crc reduces degenerate hash collision conditions.
	 */
	crcx = hammer2_icrc32(aname, len);
	crcx = crcx ^ (crcx << 16);
	key |= crcx & 0xFFFF0000U;

	/*
	 * Set bit 15.  This allows readdir to strip bit 63 so a positive
	 * 64-bit cookie/offset can always be returned, and still guarantee
	 * that the values 0x0000-0x7FFF are available for artificial entries.
	 * ('.' and '..').
	 */
	key |= 0x8000U;

	return (key);
}
