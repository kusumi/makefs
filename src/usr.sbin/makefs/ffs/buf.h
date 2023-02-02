/*	$NetBSD: buf.h,v 1.3 2001/11/02 03:12:49 lukem Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Luke Mewburn for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project by
 *      Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _FFS_BUF_H
#define	_FFS_BUF_H

#include <sys/param.h>
#include <sys/queue.h>

typedef int64_t	makefs_daddr_t; /* DragonFly */

/* DragonFly */
enum vtype	{ VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD,
		  VDATABASE, VINT };
/* DragonFly */
enum vtagtype	{
	VT_NON, VT_UFS, VT_NFS, VT_MFS, VT_UNUSED4, VT_UNUSED5, VT_UNUSED6,
	VT_UNUSED7, VT_UNUSED8, VT_NULL, VT_UNUSED10, VT_UNUSED11, VT_PROCFS,
	VT_UNUSED13, VT_ISOFS, VT_UNUSED15, VT_MSDOSFS, VT_UNUSED17, VT_VFS,
	VT_UNUSED19, VT_NTFS, VT_HPFS, VT_SMBFS, VT_UDF, VT_EXT2FS, VT_SYNTH,
	VT_HAMMER, VT_HAMMER2, VT_DEVFS, VT_TMPFS, VT_AUTOFS, VT_FUSE
};

struct componentname;
struct makefs_fsinfo;
struct ucred;

struct m_vnode {
	struct makefs_fsinfo *fs;
	void *v_data;
	enum vtype v_type; /* DragonFly */
	int v_logical; /* DragonFly */
	int v_vflushed; /* DragonFly */
	int v_malloced; /* DragonFly */
};

typedef enum buf_cmd {
	BUF_CMD_READ = 1,
	BUF_CMD_WRITE,
	BUF_CMD_FLUSH,
} buf_cmd_t;

struct m_buf {
	char *		b_data;
	long		b_bufsize;
	long		b_bcount;
	makefs_daddr_t	b_blkno; /* expanded to 64 bits */
	makefs_daddr_t	b_lblkno; /* expanded to 64 bits */
	makefs_daddr_t	b_loffset; /* DragonFly + expanded to 64 bits */
	buf_cmd_t	b_cmd; /* DragonFly */
	int		b_is_hammer2; /* DragonFly */
	struct m_vnode	*b_vp; /* DragonFly */
	struct makefs_fsinfo *b_fs;

	TAILQ_ENTRY(m_buf)	b_tailq;
};

void		bcleanup(void);
int		bread(struct m_vnode *, makefs_daddr_t, int, struct ucred *,
    struct m_buf **);
void		brelse(struct m_buf *);
int		bwrite(struct m_buf *);
struct m_buf *	getblk(struct m_vnode *, makefs_daddr_t, int, int, int, int);

#define	bdwrite(bp)	bwrite(bp)
#define	clrbuf(bp)	memset((bp)->b_data, 0, (u_int)(bp)->b_bcount)

#endif	/* _FFS_BUF_H */
