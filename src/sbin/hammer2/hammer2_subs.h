/*
 * Copyright (c) 2019 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@dragonflybsd.org>
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

#ifndef HAMMER2_HAMMER2_SUBS_H_
#define HAMMER2_HAMMER2_SUBS_H_

#include <sys/types.h>
#include <uuid/uuid.h>

#include <vfs/hammer2/hammer2_disk.h>

typedef struct hammer2_volume {
	int fd;
	int id;
	char *path;
	hammer2_off_t offset;
	hammer2_off_t size;
} hammer2_volume_t;

typedef struct hammer2_ondisk {
	int version;
	int nvolumes;
	hammer2_volume_t volumes[HAMMER2_MAX_VOLUMES];
	hammer2_off_t total_size;
	hammer2_off_t free_size;
	hammer2_uuid_t fsid;
	hammer2_uuid_t fstype;
} hammer2_ondisk_t;

/*
 * Misc functions
 */
const char *hammer2_uuid_to_str(const hammer2_uuid_t *uuid, char **strp);
const char *sizetostr(hammer2_off_t size);
hammer2_off_t check_volume(int fd);
hammer2_key_t dirhash(const char *aname, size_t len);

#define hammer2_icrc32(buf, size)	iscsi_crc32((buf), (size))
#define hammer2_icrc32c(buf, size, crc)	iscsi_crc32_ext((buf), (size), (crc))
uint32_t iscsi_crc32(const void *buf, size_t size);
uint32_t iscsi_crc32_ext(const void *buf, size_t size, uint32_t ocrc);

void hammer2_init_ondisk(hammer2_ondisk_t *fsp);
void hammer2_install_volume(hammer2_volume_t *vol, int fd, int id,
	const char *path, hammer2_off_t offset, hammer2_off_t size);
void hammer2_uninstall_volume(hammer2_volume_t *vol);
void hammer2_verify_volumes(hammer2_ondisk_t *fsp,
	const hammer2_volume_data_t *rootvoldata);
void hammer2_print_volumes(const hammer2_ondisk_t *fsp);
void hammer2_init_volumes(const char *blkdevs, int rdonly);
void hammer2_cleanup_volumes(void);

int hammer2_get_volume_fd(hammer2_off_t offset);
int hammer2_get_root_volume_fd(void);
int hammer2_get_volume_id(hammer2_off_t offset);
int hammer2_get_root_volume_id(void);
const char *hammer2_get_volume_path(hammer2_off_t offset);
const char *hammer2_get_root_volume_path(void);
hammer2_off_t hammer2_get_volume_offset(hammer2_off_t offset);
hammer2_off_t hammer2_get_root_volume_offset(void);
hammer2_off_t hammer2_get_volume_size(hammer2_off_t offset);
hammer2_off_t hammer2_get_root_volume_size(void);

hammer2_off_t hammer2_get_total_size(void);
hammer2_volume_data_t* hammer2_read_root_volume_header(void);

void hammer2_uuid_create(hammer2_uuid_t *uuid);
int hammer2_uuid_from_string(const char *str, hammer2_uuid_t *uuid);
int hammer2_uuid_to_string(const hammer2_uuid_t *uuid, char **str);
int hammer2_uuid_name_lookup(hammer2_uuid_t *uuid, const char *str);
int hammer2_uuid_addr_lookup(const hammer2_uuid_t *uuid, char **str);
int hammer2_uuid_compare(const hammer2_uuid_t *uuid1, const hammer2_uuid_t *uuid2);

#endif /* !HAMMER2_HAMMER2_SUBS_H_ */