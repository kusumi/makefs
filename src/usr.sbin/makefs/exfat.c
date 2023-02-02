/*
 * Copyright (c) 2011-2018 Andrew Nayenko
 * Copyright (c) 2022 Tomohiro Kusumi <tkusumi@netbsd.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <util.h>

#include "makefs.h"
#include "exfat/libexfat/exfat.h"
#include "exfat/mkfs/cbm.h"
#include "exfat/mkfs/fat.h"
#include "exfat/mkfs/mkexfat.h"
#include "exfat/mkfs/rootdir.h"
#include "exfat/mkfs/uct.h"
#include "exfat/mkfs/uctc.h"
#include "exfat/mkfs/vbr.h"

/* mkfs */
const struct fs_object* objects[] = {
	&vbr,
	&vbr,
	&fat,
	/* clusters heap */
	&cbm,
	&uct,
	&rootdir,
	NULL,
};

static struct {
	int sector_bits;
	int spc_bits;
	off_t volume_size;
	le16_t volume_label[EXFAT_ENAME_MAX + 1];
	uint32_t volume_serial;
	uint64_t first_sector;
} param;

int
get_sector_bits(void)
{
	return param.sector_bits;
}

int
get_spc_bits(void)
{
	return param.spc_bits;
}

off_t
get_volume_size(void)
{
	return param.volume_size;
}

const le16_t
*get_volume_label(void)
{
	return param.volume_label;
}

uint32_t
get_volume_serial(void)
{
	return param.volume_serial;
}

uint64_t
get_first_sector(void)
{
	return param.first_sector;
}

int
get_sector_size(void)
{
	return 1 << get_sector_bits();
}

int
get_cluster_size(void)
{
	return get_sector_size() << get_spc_bits();
}

static int
setup_spc_bits(int sector_bits, int user_defined, off_t volume_size)
{
	int i;

	if (user_defined != -1) {
		off_t cluster_size = 1 << sector_bits << user_defined;
		if (volume_size / cluster_size > EXFAT_LAST_DATA_CLUSTER) {
			struct exfat_human_bytes chb, vhb;
			exfat_humanize_bytes(cluster_size, &chb);
			exfat_humanize_bytes(volume_size, &vhb);
			exfat_error("cluster size %"PRIu64" %s is too small for "
					"%"PRIu64" %s volume, try -s %d",
					chb.value, chb.unit,
					vhb.value, vhb.unit,
					1 << setup_spc_bits(sector_bits, -1,
						volume_size));
			return -1;
		}
		return user_defined;
	}

	if (volume_size < 256LL * 1024 * 1024)
		return MAX(0, 12 - sector_bits);	/* 4 KB */
	if (volume_size < 32LL * 1024 * 1024 * 1024)
		return MAX(0, 15 - sector_bits);	/* 32 KB */

	for (i = 17; ; i++)	/* 128 KB or more */
		if (DIV_ROUND_UP(volume_size, 1 << i) <= EXFAT_LAST_DATA_CLUSTER)
			return MAX(0, i - sector_bits);
	assert(0);
}

static int
setup_volume_label(le16_t label[EXFAT_ENAME_MAX + 1], const char *s)
{
	memset(label, 0, (EXFAT_ENAME_MAX + 1) * sizeof(le16_t));
	if (s == NULL)
		return 0;
	return exfat_utf8_to_utf16(label, s, EXFAT_ENAME_MAX + 1, strlen(s));
}

static uint32_t
setup_volume_serial(uint32_t user_defined)
{
	struct timeval now;

	if (user_defined != 0)
		return user_defined;

	if (gettimeofday(&now, NULL) != 0) {
		exfat_error("failed to form volume id");
		return 0;
	}
	return (now.tv_sec << 20) | now.tv_usec;
}

static int
setup(struct exfat_dev *dev, int sector_bits, int spc_bits,
	const char *volume_label, uint32_t volume_serial, uint64_t first_sector)
{
	param.sector_bits = sector_bits;
	param.first_sector = first_sector;
	param.volume_size = exfat_get_size(dev);

	param.spc_bits = setup_spc_bits(sector_bits, spc_bits, param.volume_size);
	if (param.spc_bits == -1)
		return 1;

	if (setup_volume_label(param.volume_label, volume_label) != 0)
		return 1;

	param.volume_serial = setup_volume_serial(volume_serial);
	if (param.volume_serial == 0)
		return 1;

	return mkfs(dev, param.volume_size);
}

static int
logarithm2(int n)
{
	size_t i;

	for (i = 0; i < sizeof(int) * CHAR_BIT - 1; i++)
		if ((1 << i) == n)
			return i;
	return -1;
}

/* dump */
static void
print_generic_info(const struct exfat_super_block *sb)
{
	printf("Volume serial number      0x%08x\n",
		le32_to_cpu(sb->volume_serial));
	printf("FS version                       %hhu.%hhu\n",
		sb->version.major, sb->version.minor);
	printf("Sector size               %10u\n",
		SECTOR_SIZE(*sb));
	printf("Cluster size              %10u\n",
		CLUSTER_SIZE(*sb));
}

static void
print_sector_info(const struct exfat_super_block *sb)
{
	printf("Sectors count             %10"PRIu64"\n",
		le64_to_cpu(sb->sector_count));
}

static void
print_cluster_info(const struct exfat_super_block *sb)
{
	printf("Clusters count            %10u\n",
		le32_to_cpu(sb->cluster_count));
}

static void
print_other_info(const struct exfat_super_block *sb)
{
	printf("First sector              %10"PRIu64"\n",
		le64_to_cpu(sb->sector_start));
	printf("FAT first sector          %10u\n",
		le32_to_cpu(sb->fat_sector_start));
	printf("FAT sectors count         %10u\n",
		le32_to_cpu(sb->fat_sector_count));
	printf("First cluster sector      %10u\n",
		le32_to_cpu(sb->cluster_sector_start));
	printf("Root directory cluster    %10u\n",
		le32_to_cpu(sb->rootdir_cluster));
	printf("Volume state                  0x%04hx\n",
		le16_to_cpu(sb->volume_state));
	printf("FATs count                %10hhu\n",
		sb->fat_count);
	printf("Drive number                    0x%02hhx\n",
		sb->drive_no);
	printf("Allocated space           %9hhu%%\n",
		sb->allocated_percent);
}

static void
dump_sectors(struct exfat *ef)
{
	off_t a = 0, b = 0;

	printf("Used sectors ");
	while (exfat_find_used_sectors(ef, &a, &b) == 0)
		printf(" %"PRIu64"-%"PRIu64, a, b);
	puts("");
}

static int
dump_full(const char *spec, bool used_sectors)
{
	struct exfat ef;
	uint32_t free_clusters;
	uint64_t free_sectors;

	if (exfat_mount(&ef, spec, "ro") != 0)
		return 1;

	free_clusters = exfat_count_free_clusters(&ef);
	free_sectors = (uint64_t) free_clusters << ef.sb->spc_bits;

	printf("Volume label         %15s\n", exfat_get_label(&ef));
	print_generic_info(ef.sb);
	print_sector_info(ef.sb);
	printf("Free sectors              %10"PRIu64"\n", free_sectors);
	print_cluster_info(ef.sb);
	printf("Free clusters             %10u\n", free_clusters);
	print_other_info(ef.sb);
	if (used_sectors)
		dump_sectors(&ef);

	exfat_unmount(&ef);
	return 0;
}

/* fsck */
#define exfat_debug(format, ...) do {} while (0)

static uint64_t files_count;
static uint64_t directories_count;

static int
nodeck(struct exfat *ef, struct exfat_node *node)
{
	const cluster_t cluster_size = CLUSTER_SIZE(*ef->sb);
	cluster_t clusters = DIV_ROUND_UP(node->size, cluster_size);
	cluster_t c = node->start_cluster;
	int rc = 0;

	while (clusters--) {
		if (CLUSTER_INVALID(*ef->sb, c)) {
			char name[EXFAT_UTF8_NAME_BUFFER_MAX];
			exfat_get_name(node, name);
			exfat_error("file '%s' has invalid cluster 0x%x",
				name, c);
			rc = 1;
			break;
		}
		if (BMAP_GET(ef->cmap.chunk, c - EXFAT_FIRST_DATA_CLUSTER) == 0) {
			char name[EXFAT_UTF8_NAME_BUFFER_MAX];
			exfat_get_name(node, name);
			exfat_error("cluster 0x%x of file '%s' is not allocated",
				c, name);
			rc = 1;
		}
		c = exfat_next_cluster(ef, node, c);
	}
	return rc;
}

static void
dirck(struct exfat *ef, const char *path)
{
	struct exfat_node *parent;
	struct exfat_node *node;
	struct exfat_iterator it;
	int rc;
	size_t path_length;
	char *entry_path;

	if (exfat_lookup(ef, &parent, path) != 0)
		exfat_bug("directory '%s' is not found", path);
	if (!(parent->attrib & EXFAT_ATTRIB_DIR))
		exfat_bug("'%s' is not a directory (%#hx)",
			path, parent->attrib);
	if (nodeck(ef, parent) != 0) {
		exfat_put_node(ef, parent);
		return;
	}

	path_length = strlen(path);
	entry_path = malloc(path_length + 1 + EXFAT_UTF8_NAME_BUFFER_MAX);
	if (entry_path == NULL) {
		exfat_put_node(ef, parent);
		exfat_error("out of memory");
		return;
	}
	strcpy(entry_path, path);
	strcat(entry_path, "/");

	rc = exfat_opendir(ef, parent, &it);
	if (rc != 0) {
		free(entry_path);
		exfat_put_node(ef, parent);
		return;
	}
	while ((node = exfat_readdir(&it))) {
		exfat_get_name(node, entry_path + path_length + 1);
		exfat_debug("%s: %s, %"PRIu64" bytes, cluster %u", entry_path,
				node->is_contiguous ? "contiguous" : "fragmented",
				node->size, node->start_cluster);
		if (node->attrib & EXFAT_ATTRIB_DIR) {
			directories_count++;
			dirck(ef, entry_path);
		} else {
			files_count++;
			nodeck(ef, node);
		}
		exfat_flush_node(ef, node);
		exfat_put_node(ef, node);
	}
	exfat_closedir(ef, &it);
	exfat_flush_node(ef, parent);
	exfat_put_node(ef, parent);
	free(entry_path);
}

static void
fsck(struct exfat *ef, const char *spec, const char *options)
{
	if (exfat_mount(ef, spec, options) != 0) {
		fputs("File system checking stopped\n", stdout);
		return;
	}

	exfat_print_info(ef->sb, exfat_count_free_clusters(ef));
	exfat_soil_super_block(ef);
	dirck(ef, "");
	exfat_unmount(ef);

	printf("Total %"PRIu64" directories and %"PRIu64" files\n",
		directories_count, files_count);
	fputs("File system checking finished\n", stdout);
}

/* makefs */
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

/* see src/gpl/github.com/relan/exfat/mkfs/mkexfatfs.8 */
void
exfat_prep_opts(fsinfo_t *fsopts)
{
	struct exfat_options *exfat_opts = ecalloc(1, sizeof(*exfat_opts));
	const option_t fs_options[] = {
		/* msdos compatible options */
		{ 'C', "create_size", &exfat_opts->create_size, OPT_INT64, 0, LONG_MAX,
		  "Image size to create, e.g. 1g for 1GiB image file.", },
		/* src/gpl/github.com/relan/exfat/mkfs compatible options */
		{ 'n', "volume_name", &exfat_opts->volume_label, OPT_STRPTR, 0, 0,
		  "Volume name (label), up to 15 characters. "
		  "By default no label is set.", },
		{ 'i', "volume_id", NULL, OPT_STRBUF, 0, 0,
		  "A 32-bit hexadecimal number. "
		  "By default a value based on current time is set.", },
		{ 'p', "part_first_sector", NULL, OPT_STRBUF, 0, 0,
		  "First sector of the partition starting from the beginning of the whole disk. "
		  "exFAT super block has a field for this value but in fact it's optional and does not affect anything. "
		  "Default is 0.", },
		{ 's', "sectors_per_cluster", NULL, OPT_STRBUF, 0, 0,
		  "Number of physical sectors per cluster (cluster is an allocation unit in exFAT). "
		  "Must be a power of 2, i.e. 1, 2, 4, 8, etc. "
		  "Cluster size can not exceed 32 MB. "
		  "Default cluster sizes are: 4 KB if volume size is less than 256 MB, 32 KB if volume size is from 256 MB to 32 GB, 128 KB if volume size is 32 GB or larger.", },
		{ .name = NULL },
	};

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
		printf("ftruncate(%d, 0x%lx)\n", fd, size);
		if (ftruncate(fd, size) == -1)
			err(1, "ftruncate failed");
	} else {
		printf("posix_fallocate(%d, 0, 0x%lx)\n", fd, size);
		if (posix_fallocate(fd, 0, size) == -1)
			err(1, "posix_fallocate failed");
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
	printf("create_size=0x%lx\n", exfat_opts->create_size);
	printf("volume_label=%s\n", exfat_opts->volume_label);
	printf("volume_serial=0x%x\n", exfat_opts->volume_serial);
	printf("first_sector=%ld\n", exfat_opts->first_sector);
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
