/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Daniel Lezcano <daniel.lezcano at free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __LXC_STORAGE_H
#define __LXC_STORAGE_H

#include "config.h"
#include <stdint.h>
#include <sys/mount.h>

#include <lxc/lxccontainer.h>

#if IS_BIONIC
#include <../include/lxcmntent.h>
#else
#include <mntent.h>
#endif

#ifndef MS_DIRSYNC
#define MS_DIRSYNC 128
#endif

#ifndef MS_REC
#define MS_REC 16384
#endif

#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif

#ifndef MS_SLAVE
#define MS_SLAVE (1 << 19)
#endif

#ifndef MS_RELATIME
#define MS_RELATIME (1 << 21)
#endif

#ifndef MS_STRICTATIME
#define MS_STRICTATIME (1 << 24)
#endif

#define DEFAULT_FS_SIZE 1073741824
#define DEFAULT_FSTYPE "ext4"

#define LXC_STORAGE_INTERNAL_OVERLAY_RESTORE  (1 << 6)

struct lxc_storage;

struct lxc_storage_ops {
	/* detect whether path is of this bdev type */
	bool (*detect)(const char *path);

	/* mount requires src and dest to be set. */
	int (*mount)(struct lxc_storage *bdev);
	int (*umount)(struct lxc_storage *bdev);
	int (*destroy)(struct lxc_storage *bdev);
	int (*create)(struct lxc_storage *bdev, const char *dest, const char *n,
		      struct bdev_specs *specs);
	/* given original mount, rename the paths for cloned container */
	int (*clone_paths)(struct lxc_storage *orig, struct lxc_storage *new,
			   const char *oldname, const char *cname,
			   const char *oldpath, const char *lxcpath, int snap,
			   uint64_t newsize, struct lxc_conf *conf);
	bool (*copy)(struct lxc_conf *conf, struct lxc_storage *orig,
		     struct lxc_storage *new, uint64_t newsize);
	bool (*snapshot)(struct lxc_conf *conf, struct lxc_storage *orig,
			 struct lxc_storage *new, uint64_t newsize);
	bool can_snapshot;
	bool can_backup;
};

/* When lxc is mounting a rootfs, then src will be the "lxc.rootfs.path" value,
 * dest will be the mount dir (i.e. "<libdir>/lxc")  When clone or create is
 * doing so, then dest will be "<lxcpath>/<lxcname>/rootfs", since we may need
 * to rsync from one to the other.
 */
struct lxc_storage {
	const struct lxc_storage_ops *ops;
	const char *type;
	char *src;
	char *dest;
	char *mntopts;
	/* Turn the following into a union if need be. */
	/* lofd is the open fd for the mounted loopback file. */
	int lofd;
	/* index for the connected nbd device. */
	int nbd_idx;
	int flags;
};

extern bool storage_is_dir(struct lxc_conf *conf, const char *path);
extern bool storage_can_backup(struct lxc_conf *conf);

/* Instantiate a lxc_storage object. The src is used to determine which blockdev
 * type this should be. The dst and data are optional, and will be used in case
 * of mount/umount.
 *
 * The source will be "dir:/var/lib/lxc/c1" or "lvm:/dev/lxc/c1". For other
 * backing stores, this will allow additional options. In particular,
 * "overlayfs:/var/lib/lxc/canonical/rootfs:/var/lib/lxc/c1/delta" will mean use
 * /var/lib/lxc/canonical/rootfs as lower dir, and /var/lib/lxc/c1/delta as the
 * upper, writeable layer.
 */
extern struct lxc_storage *storage_init(struct lxc_conf *conf, const char *src,
					const char *dst, const char *data);

extern struct lxc_storage *storage_copy(struct lxc_container *c0,
					const char *cname, const char *lxcpath,
					const char *bdevtype, int flags,
					const char *bdevdata, uint64_t newsize,
					bool *needs_rdep);
extern struct lxc_storage *storage_create(const char *dest, const char *type,
					  const char *cname,
					  struct bdev_specs *specs);
extern void storage_put(struct lxc_storage *bdev);
extern bool storage_destroy(struct lxc_conf *conf);

extern int storage_destroy_wrapper(void *data);
extern bool rootfs_is_blockdev(struct lxc_conf *conf);
extern char *lxc_storage_get_path(char *src, const char *prefix);

#endif /* #define __LXC_STORAGE_H */
