/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"
#include "libxfs.h"

static cmdinfo_t stat_cmd;
static cmdinfo_t statfs_cmd;

off64_t
filesize(void)
{
	struct stat	st;

	if (fstat(file->fd, &st) < 0) {
		perror("fstat");
		return -1;
	}
	return st.st_size;
}

static char *
filetype(mode_t mode)
{
	switch (mode & S_IFMT) {
	case S_IFSOCK:
		return _("socket");
	case S_IFDIR:
		return _("directory");
	case S_IFCHR:
		return _("char device");
	case S_IFBLK:
		return _("block device");
	case S_IFREG:
		return _("regular file");
	case S_IFLNK:
		return _("symbolic link");
	case S_IFIFO:
		return _("fifo");
	}
	return NULL;
}

int
stat_f(
	int		argc,
	char		**argv)
{
	struct dioattr	dio;
	struct fsxattr	fsx, fsxa;
	struct stat	st;
	int		verbose = (argc == 2 && !strcmp(argv[1], "-v"));

	printf(_("fd.path = \"%s\"\n"), file->name);
	printf(_("fd.flags = %s,%s,%s%s%s%s%s\n"),
		file->flags & IO_OSYNC ? _("sync") : _("non-sync"),
		file->flags & IO_DIRECT ? _("direct") : _("non-direct"),
		file->flags & IO_READONLY ? _("read-only") : _("read-write"),
		file->flags & IO_REALTIME ? _(",real-time") : "",
		file->flags & IO_APPEND ? _(",append-only") : "",
		file->flags & IO_NONBLOCK ? _(",non-block") : "",
		file->flags & IO_TMPFILE ? _(",tmpfile") : "");
	if (fstat(file->fd, &st) < 0) {
		perror("fstat");
	} else {
		printf(_("stat.ino = %lld\n"), (long long)st.st_ino);
		printf(_("stat.type = %s\n"), filetype(st.st_mode));
		printf(_("stat.size = %lld\n"), (long long)st.st_size);
		printf(_("stat.blocks = %lld\n"), (long long)st.st_blocks);
		if (verbose) {
			printf(_("stat.atime = %s"), ctime(&st.st_atime));
			printf(_("stat.mtime = %s"), ctime(&st.st_mtime));
			printf(_("stat.ctime = %s"), ctime(&st.st_ctime));
		}
	}
	if (file->flags & IO_FOREIGN)
		return 0;
	if ((xfsctl(file->name, file->fd, FS_IOC_FSGETXATTR, &fsx)) < 0 ||
	    (xfsctl(file->name, file->fd, XFS_IOC_FSGETXATTRA, &fsxa)) < 0) {
		perror("FS_IOC_FSGETXATTR");
	} else {
		printf(_("fsxattr.xflags = 0x%x "), fsx.fsx_xflags);
		printxattr(fsx.fsx_xflags, verbose, 0, file->name, 1, 1);
		printf(_("fsxattr.projid = %u\n"), fsx.fsx_projid);
		printf(_("fsxattr.extsize = %u\n"), fsx.fsx_extsize);
		printf(_("fsxattr.cowextsize = %u\n"), fsx.fsx_cowextsize);
		printf(_("fsxattr.nextents = %u\n"), fsx.fsx_nextents);
		printf(_("fsxattr.naextents = %u\n"), fsxa.fsx_nextents);
	}
	if ((xfsctl(file->name, file->fd, XFS_IOC_DIOINFO, &dio)) < 0) {
		perror("XFS_IOC_DIOINFO");
	} else {
		printf(_("dioattr.mem = 0x%x\n"), dio.d_mem);
		printf(_("dioattr.miniosz = %u\n"), dio.d_miniosz);
		printf(_("dioattr.maxiosz = %u\n"), dio.d_maxiosz);
	}
	return 0;
}

static int
statfs_f(
	int			argc,
	char			**argv)
{
	struct xfs_fsop_counts	fscounts;
	struct xfs_fsop_geom	fsgeo;
	struct statfs		st;

	printf(_("fd.path = \"%s\"\n"), file->name);
	if (platform_fstatfs(file->fd, &st) < 0) {
		perror("fstatfs");
	} else {
		printf(_("statfs.f_bsize = %lld\n"), (long long) st.f_bsize);
		printf(_("statfs.f_blocks = %lld\n"), (long long) st.f_blocks);
		printf(_("statfs.f_bavail = %lld\n"), (long long) st.f_bavail);
		printf(_("statfs.f_files = %lld\n"), (long long) st.f_files);
		printf(_("statfs.f_ffree = %lld\n"), (long long) st.f_ffree);
	}
	if (file->flags & IO_FOREIGN)
		return 0;
	if ((xfsctl(file->name, file->fd, XFS_IOC_FSGEOMETRY_V1, &fsgeo)) < 0) {
		perror("XFS_IOC_FSGEOMETRY_V1");
	} else {
		printf(_("geom.bsize = %u\n"), fsgeo.blocksize);
		printf(_("geom.agcount = %u\n"), fsgeo.agcount);
		printf(_("geom.agblocks = %u\n"), fsgeo.agblocks);
		printf(_("geom.datablocks = %llu\n"),
			(unsigned long long) fsgeo.datablocks);
		printf(_("geom.rtblocks = %llu\n"),
			(unsigned long long) fsgeo.rtblocks);
		printf(_("geom.rtextents = %llu\n"),
			(unsigned long long) fsgeo.rtextents);
		printf(_("geom.rtextsize = %u\n"), fsgeo.rtextsize);
		printf(_("geom.sunit = %u\n"), fsgeo.sunit);
		printf(_("geom.swidth = %u\n"), fsgeo.swidth);
	}
	if ((xfsctl(file->name, file->fd, XFS_IOC_FSCOUNTS, &fscounts)) < 0) {
		perror("XFS_IOC_FSCOUNTS");
	} else {
		printf(_("counts.freedata = %llu\n"),
			(unsigned long long) fscounts.freedata);
		printf(_("counts.freertx = %llu\n"),
			(unsigned long long) fscounts.freertx);
		printf(_("counts.freeino = %llu\n"),
			(unsigned long long) fscounts.freeino);
		printf(_("counts.allocino = %llu\n"),
			(unsigned long long) fscounts.allocino);
	}
	return 0;
}

void
stat_init(void)
{
	stat_cmd.name = "stat";
	stat_cmd.cfunc = stat_f;
	stat_cmd.argmin = 0;
	stat_cmd.argmax = 1;
	stat_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	stat_cmd.args = _("[-v]");
	stat_cmd.oneline = _("statistics on the currently open file");

	statfs_cmd.name = "statfs";
	statfs_cmd.cfunc = statfs_f;
	statfs_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	statfs_cmd.oneline =
		_("statistics on the filesystem of the currently open file");

	add_command(&stat_cmd);
	add_command(&statfs_cmd);
}
