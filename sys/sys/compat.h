/*
 * Copyright (c) 2022 Tomohiro Kusumi <tkusumi@netbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MAKEFS_COMPAT_H
#define _MAKEFS_COMPAT_H

#define _GNU_SOURCE /* vasprintf(3) */

#include <stdint.h> /* uintX_t */
#include <stddef.h> /* size_t, offsetof() */

/* sys/sys/cdefs.h */
#define __FBSDID(s)	struct __hack
#define __RCSID(s)	struct __hack
#define __SCCSID(s)	struct __hack

#ifndef __printflike
#define __printflike(fmtarg, firstvararg)
#endif
#ifndef __predict_false
#define __predict_false(exp)	(exp)
#endif
#ifndef __offsetof
#define __offsetof(type, field)	offsetof(type, field)
#endif
#ifndef __restrict
#define __restrict
#endif

#if __GNUC_PREREQ(2, 7)
/*
 * Note that some Linux headers have a struct field named __unused.
 * e.g. <linux/sysctl.h> included by <sys/sysctl.h>.
 * e.g. <bits/stat.h> included by <sys/stat.h>.
 * <sys/compat.h> should be included after such headers that cause conflicts.
 */
#ifndef __unused
#define __unused	__attribute__((unused))
#endif
#ifndef __dead
#define __dead		__attribute__((__noreturn__))
#endif
#ifndef __dead2
#define __dead2		__attribute__((__noreturn__))
#endif
#ifndef __packed
#define __packed	__attribute__((__packed__))
#endif
#else
#ifndef __unused
#define __unused
#endif
#ifndef __dead
#define __dead
#endif
#ifndef __dead2
#define __dead2
#endif
#ifndef __packed
#define __packed
#endif
#endif

/* sys/sys/param.h */
#define MAXPHYS		(1024 * 1024)
#define MAXBSIZE	65536	/* must be power of 2 */

#ifndef nitems
#define nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif
#ifndef roundup
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif
#ifndef roundup2
#define roundup2(x, y)	roundup(x, y)
#endif

/* lib/libc/include/libc_private.h */
extern const char *__progname;

/* include/stdlib.h */
const char *getprogname(void);
void setprogname(const char *);

/* lib/libnetbsd/stdlib.h */
long long strsuftoll(const char *, const char *, long long, long long);

/* include/err.h */
void errc(int, int, const char *, ...);

/* include/string.h */
size_t strlcat(char * __restrict, const char * __restrict, size_t);
size_t strlcpy(char * __restrict, const char * __restrict, size_t);

#endif /* _MAKEFS_COMPAT_H */
