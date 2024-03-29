/*	$FreeBSD$	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2012 SRI International
 * Copyright (c) 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LIBNETBSD_UTIL_H_
#define _LIBNETBSD_UTIL_H_

#include <sys/types.h>
//#include <libutil.h>
#include <stdio.h>

#if 0
void	(*esetfunc(void (*)(int, const char *, ...)))(int, const char *, ...);
size_t	 estrlcpy(char *, const char *, size_t);
size_t	 estrlcat(char *, const char *, size_t);
#endif
char	*estrdup(const char *);
#if 0
char	*estrndup(const char *, size_t);
#endif
void	*emalloc(size_t);
void	*ecalloc(size_t, size_t);
void	*erealloc(void *, size_t);
#if 0
FILE	*efopen(const char *, const char *);
int	 easprintf(char ** __restrict, const char * __restrict, ...)
	    __printflike(2, 3);
int	 evasprintf(char ** __restrict, const char * __restrict, __va_list)
	    __printflike(2, 0);
char	*flags_to_string(u_long flags, const char *def);
int	 sockaddr_snprintf(char *, size_t, const char *,
			   const struct sockaddr *);
int	 string_to_flags(char **stringp, u_long *setp, u_long *clrp);
#endif

long long
strsuftoll(const char *desc, const char *val,
    long long min, long long max);

#endif
