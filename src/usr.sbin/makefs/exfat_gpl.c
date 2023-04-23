/*
 * Copyright (c) 2011-2018 Andrew Nayenko
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

#include <inttypes.h>
#include <limits.h>

#include "exfat/libexfat/exfat.h"
#include "exfat/mkfs/cbm.h"
#include "exfat/mkfs/fat.h"
#include "exfat/mkfs/mkexfat.h"
#include "exfat/mkfs/rootdir.h"
#include "exfat/mkfs/uct.h"
#include "exfat/mkfs/uctc.h"
#include "exfat/mkfs/vbr.h"

/* see src/gpl/github.com/relan/exfat/mkfs/mkexfatfs.8 */
static option_t fs_options[] = {
	/* msdos compatible options */
	{ 'C', "create_size", NULL, OPT_INT64, 0, LONG_MAX,
	  "Image size to create, e.g. 1g for 1GiB image file.", },
	/* src/gpl/github.com/relan/exfat/mkfs compatible options */
	{ 'n', "volume_name", NULL, OPT_STRPTR, 0, 0,
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
