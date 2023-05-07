/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2022 Tomohiro Kusumi <tkusumi@netbsd.org>
 * Copyright (c) 2011-2022 The DragonFly Project.  All rights reserved.
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

//#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <err.h>
#include <assert.h>
#include <util.h>

#include "makefs.h"
#include "./exfat_gpl.c"

static int exfat_populate_dir(struct exfat *, const char *, fsnode *, fsnode *,
    fsinfo_t *, int);
static int exfat_write_file(struct exfat *, struct exfat_node *, const char *,
    fsnode *);

struct exfat_options {
	int64_t create_size;
	char *volume_label;
	uint32_t volume_serial;
	uint64_t first_sector;
	int32_t spc_bits;
};

void
exfat_prep_opts(fsinfo_t *fsopts)
{
	struct exfat_options *exfat_opts = ecalloc(1, sizeof(*exfat_opts));

	assert(fs_options[0].letter == 'C');
	assert(fs_options[0].value == NULL);
	fs_options[0].value = &exfat_opts->create_size;

	assert(fs_options[1].letter == 'n');
	assert(fs_options[1].value == NULL);
	fs_options[1].value = &exfat_opts->volume_label;

	exfat_opts->create_size = 0;
	exfat_opts->volume_label = NULL;
	exfat_opts->volume_serial = 0;
	exfat_opts->first_sector = 0;
	exfat_opts->spc_bits = -1;

	fsopts->fs_specific = exfat_opts;
	fsopts->fs_options = copy_opts(fs_options);
}

void
exfat_cleanup_opts(fsinfo_t *fsopts)
{
	free(fsopts->fs_specific);
	free(fsopts->fs_options);
}

int
exfat_parse_opts(const char *option, fsinfo_t *fsopts)
{
	struct exfat_options *exfat_opts = fsopts->fs_specific;
	option_t *fs_options = fsopts->fs_options;

	assert(option != NULL);
	assert(fsopts != NULL);
	assert(exfat_opts != NULL);

	if (debug & DEBUG_FS_PARSE_OPTS)
		printf("got option \"%s\"\n", option);

	char buf[1024];
	int i = set_option(fs_options, option, buf, sizeof(buf));
	if (i == -1)
		return i;

	char c = fs_options[i].letter;
	switch (c) {
	case 'i':
		exfat_opts->volume_serial = strtol(buf, NULL, 16);
		break;
	case 'p':
		exfat_opts->first_sector = strtoll(buf, NULL, 10);
		break;
	case 's':
		exfat_opts->spc_bits = logarithm2(atoi(buf));
		if (exfat_opts->spc_bits < 0)
			errx(1, "invalid \"-o %c\" option value '%s'", c, buf);
		break;
	default:
		break;
	}

	return 1;
}

static void
print_line(void)
{
	printf("----------------------------------------\n");
}

static int
exfat_init_image(const char *image, fsinfo_t *fsopts)
{
	struct exfat_options *exfat_opts = fsopts->fs_specific;

	if (fsopts->offset != 0)
		errx(1, "-O option unsupported");

	fsopts->size = fsopts->maxsize;
	exfat_opts->create_size = MAX(exfat_opts->create_size, fsopts->size);

	int64_t size = exfat_opts->create_size;
	if (size <= 0)
		errx(1, "image file size not specified, use -s or "
		    "\"-o create_size\" option");

	int fd = open(image, O_CREAT|O_WRONLY|O_TRUNC, 0666);
	if (fd == -1)
		err(1, "failed to open %s", image);

	if (fsopts->sparse) {
		printf("ftruncate(%d, 0x%llx)\n", fd, (long long)size);
		if (ftruncate(fd, size) == -1)
			err(1, "ftruncate failed");
	} else {
#ifdef __OpenBSD__
		errx(1, "OpenBSD does not support posix_fallocate, use sparse");
#else
		printf("posix_fallocate(%d, 0, 0x%lx)\n", fd, size);
		if (posix_fallocate(fd, 0, size) == -1)
			err(1, "posix_fallocate failed");
#endif
	}
	close(fd);

	return 0;
}

void
exfat_makefs(const char *image, const char *dir, fsnode *root, fsinfo_t *fsopts)
{
	struct exfat_options *exfat_opts = fsopts->fs_specific;
	struct exfat ef;
	struct timeval start;

	assert(image != NULL);
	assert(dir != NULL);
	assert(root != NULL);
	assert(fsopts != NULL);

	printf("image=\"%s\" directory=\"%s\"\n", image, dir);
	printf("create_size=0x%llx\n", (long long)exfat_opts->create_size);
	printf("volume_label=%s\n", exfat_opts->volume_label);
	printf("volume_serial=0x%x\n", exfat_opts->volume_serial);
	printf("first_sector=%lld\n", (long long)exfat_opts->first_sector);
	printf("spc_bits=%d\n", exfat_opts->spc_bits);

	if (exfat_init_image(image, fsopts))
		errx(1, "failed to initialize image %s", image);
	print_line();

	/* mkfs */
	TIMER_START(start);
	printf("exFAT mkfs %s\n", image);
	struct exfat_dev *dev = exfat_open(image, EXFAT_MODE_RW);
	if (dev == NULL)
		errx(1, "exFAT open %s failed", image);
	if (setup(dev, 9, exfat_opts->spc_bits, exfat_opts->volume_label,
		exfat_opts->volume_serial, exfat_opts->first_sector))
		errx(1, "exFAT setup %s failed", image);
	if (exfat_close(dev))
		errx(1, "exFAT close %s failed", image);
	TIMER_RESULTS(start, "exFAT mkfs");
	print_line();

	/* dump */
	TIMER_START(start);
	printf("exFAT dump 1st %s\n", image);
	dump_full(image, true);
	TIMER_RESULTS(start, "exFAT dump");
	print_line();

	/* fsck */
	TIMER_START(start);
	printf("exFAT fsck 1st %s\n", image);
	fsck(&ef, image, "repair=0,ro");
	if (exfat_errors)
		errx(1, "exFAT fsck failed: %d errors, %d fixed",
		    exfat_errors, exfat_errors_fixed);
	TIMER_RESULTS(start, "exFAT fsck");
	print_line();

	/* populate */
	printf("exFAT populate %s\n", image);
	TIMER_START(start);
	if (exfat_mount(&ef, image, "") != 0)
		errx(1, "exFAT mount %s failed", image);
	if (exfat_populate_dir(&ef, dir, root, root, fsopts, 0) == -1)
		errx(1, "exFAT populate %s failed", image);
	exfat_unmount(&ef);
	TIMER_RESULTS(start, "exfat_populate_dir");
	print_line();

	/* dump */
	TIMER_START(start);
	printf("exFAT dump 2nd %s\n", image);
	dump_full(image, true);
	TIMER_RESULTS(start, "exFAT dump");
	print_line();

	/* fsck */
	TIMER_START(start);
	printf("exFAT fsck 2nd %s\n", image);
	fsck(&ef, image, "repair=0,ro");
	if (exfat_errors)
		errx(1, "exFAT fsck failed: %d errors, %d fixed",
		    exfat_errors, exfat_errors_fixed);
	TIMER_RESULTS(start, "exFAT fsck");
	print_line();

	printf("successfully created exFAT image %s\n", image);
}

static void
exfat_print(const char *p, bool isdir, int depth)
{
	if (debug & DEBUG_FS_POPULATE) {
		if (1) {
			int indent = depth * 2;
			printf("%*.*s", indent, indent, "");
			printf("%s %s\n", isdir ? "dir" : "reg", p);
		} else {
			printf("%c", isdir ? 'd' : 'r');
			fflush(stdout);
		}
	}
}

static int
exfat_populate_dir(struct exfat *ef, const char *dir, fsnode *root,
    fsnode *parent, fsinfo_t *fsopts, int depth)
{
	assert(ef != NULL);
	assert(dir != NULL);
	assert(root != NULL);
	assert(parent != NULL);
	assert(fsopts != NULL);

	/* assert root directory */
	assert(S_ISDIR(root->type));
	assert(!strcmp(root->name, "."));
	assert(!root->child);
	assert(!root->parent || root->parent->child == root);

	struct stat st;
	if (stat(dir, &st) == -1)
		err(1, "no such path %s", dir);
	if (!S_ISDIR(st.st_mode))
		errx(1, "no such dir %s", dir);

	for (fsnode *cur = root->next; cur != NULL; cur = cur->next) {
		/* construct source path */
		char f[MAXPATHLEN]; /* avoid stack ? */
		int ret = snprintf(f, sizeof(f), "%s/%s", dir, cur->name);
		if ((size_t)ret >= sizeof(f))
			errx(1, "path %s too long", f);

		struct stat st;
		if (stat(f, &st) == -1)
			err(1, "no such file %s", f);

		/* get pointer to exFAT path */
		if (root == parent)
			assert(!strcmp(dir, cur->root));
		size_t baselen = strlen(cur->root);
		if (baselen + 1 >= strlen(f))
			errx(1, "baselen %ld too long", baselen);
		char *p = &f[baselen + 1];
		assert(strlen(p) > 0);

		/* update node state */
		if ((cur->inode->flags & FI_ALLOCATED) == 0) {
			cur->inode->flags |= FI_ALLOCATED;
			if (cur != root) {
				fsopts->curinode++;
				cur->inode->ino = fsopts->curinode;
				cur->parent = parent;
			}
		}

		if (cur->inode->flags & FI_WRITTEN)
			continue; /* hard link */
		cur->inode->flags |= FI_WRITTEN;

		/* if directory, mkdir and recurse */
		if (S_ISDIR(cur->type)) {
			assert(cur->child);
			exfat_print(p, true, depth);

			ret = exfat_mkdir(ef, p);
			if (ret < 0)
				errx(1, "exfat_mkdir(\"%s\") failed: %s",
				    p, strerror(-ret));
			if (exfat_populate_dir(ef, f, cur->child, cur, fsopts,
			    depth + 1) == -1)
				errx(1, "%s", f);
			continue;
		}

		/* if regular file, creat and write its data */
		if (S_ISREG(cur->type)) {
			assert(cur->contents == NULL);
			assert(cur->child == NULL);
			exfat_print(p, false, depth);

			ret = exfat_mknod(ef, p);
			if (ret < 0)
				errx(1, "exfat_mknod(\"%s\") failed: %s",
				    p, strerror(-ret));

			struct exfat_node *en = NULL;
			ret = exfat_lookup(ef, &en, p);
			if (ret < 0)
				errx(1, "exfat_lookup(\"%s\") failed: %s",
				    p, strerror(-ret));
			assert(en != NULL);

			ret = exfat_write_file(ef, en, f, cur);
			if (ret < 0)
				errx(1, "exfat_write_file(\"%s\") failed: %s",
				    p, strerror(-ret));
			exfat_put_node(ef, en);
			continue;
		}

		/* ignore other types unsupported by exFAT */
		printf("ignore %s 0%o\n", f, cur->type);
	}

	return 0;
}

static int
exfat_write_file(struct exfat *ef, struct exfat_node *en, const char *path,
    fsnode *node)
{
	struct stat *st = &node->inode->st;
	int cluster_size = CLUSTER_SIZE(*ef->sb);

	size_t nsize = st->st_size;
	if (nsize == 0)
		return 0;
	/* XXX check nsize vs maximum file size */

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		err(1, "failed to open %s", path);

	char *m = mmap(0, nsize, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
	if (m == MAP_FAILED)
		err(1, "failed to mmap %s", path);
	close(fd);

	size_t offset;
	int ret;
	for (offset = 0; offset < nsize; ) {
		int size = MIN(nsize - offset, cluster_size);
		ret = exfat_generic_pwrite(ef, en, m + offset, size, offset);
		if (ret < 0)
			errx(1, "failed to pwrite %s node: %s",
			    path, strerror(-ret));
		else if (ret != size)
			errx(1, "wrote %d bytes vs expected %d bytes",
			    ret, size);
		offset += size;
	}
	munmap(m, nsize);

	ret = exfat_flush_node(ef, en);
	if (ret < 0)
		errx(1, "failed to flush %s node: %s", path, strerror(-ret));

	return 0;
}
