/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * Copyright (c) 2012 Red Hat, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include <sys/mman.h>
#include "init.h"
#include "space.h"

static cmdinfo_t print_cmd;

fileio_t	*filetable;
int		filecount;
fileio_t	*file;

static void
print_fileio(
	fileio_t	*file,
	int		index,
	int		braces)
{
	printf(_("%c%03d%c %-14s (%s,%s,%s%s%s)\n"),
		braces? '[' : ' ', index, braces? ']' : ' ', file->name,
		file->flags & O_SYNC ? _("sync") : _("non-sync"),
		file->flags & O_DIRECT ? _("direct") : _("non-direct"),
		file->flags & O_RDONLY ? _("read-only") : _("read-write"),
		file->flags & O_APPEND ? _(",append-only") : "",
		file->flags & O_NONBLOCK ? _(",non-block") : "");
}

int
filelist_f(void)
{
	int		i;

	for (i = 0; i < filecount; i++)
		print_fileio(&filetable[i], i, &filetable[i] == file);
	return 0;
}

static int
print_f(
	int		argc,
	char		**argv)
{
	filelist_f();
	return 0;
}

int
openfile(
	char		*path,
	xfs_fsop_geom_t	*geom,
	int		flags,
	mode_t		mode)
{
	int		fd;

	fd = open(path, flags, mode);
	if (fd < 0) {
		if ((errno == EISDIR) && (flags & O_RDWR)) {
			/* make it as if we asked for O_RDONLY & try again */
			flags &= ~O_RDWR;
			flags |= O_RDONLY;
			fd = open(path, flags, mode);
			if (fd < 0) {
				perror(path);
				return -1;
			}
		} else {
			perror(path);
			return -1;
		}
	}

	if (xfsctl(path, fd, XFS_IOC_FSGEOMETRY, geom) < 0) {
		perror("XFS_IOC_FSGEOMETRY");
		close(fd);
		return -1;
	}
	return fd;
}

int
addfile(
	char		*name,
	int		fd,
	xfs_fsop_geom_t	*geometry,
	int		flags)
{
	char		*filename;

	filename = strdup(name);
	if (!filename) {
		perror("strdup");
		close(fd);
		return -1;
	}

	/* Extend the table of currently open files */
	filetable = (fileio_t *)realloc(filetable,	/* growing */
					++filecount * sizeof(fileio_t));
	if (!filetable) {
		perror("realloc");
		filecount = 0;
		free(filename);
		close(fd);
		return -1;
	}

	/* Finally, make this the new active open file */
	file = &filetable[filecount - 1];
	file->fd = fd;
	file->flags = flags;
	file->name = filename;
	file->geom = *geometry;
	return 0;
}

void
file_init(void)
{
	print_cmd.name = "print";
	print_cmd.altname = "p";
	print_cmd.cfunc = print_f;
	print_cmd.argmin = 0;
	print_cmd.argmax = 0;
	print_cmd.flags = CMD_FLAG_GLOBAL;
	print_cmd.oneline = _("list current open files");

	add_command(&print_cmd);
}
