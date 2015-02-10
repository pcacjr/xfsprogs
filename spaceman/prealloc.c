/*
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
#include <xfs/xfs_types.h>
#include <xfs/command.h>
#include "input.h"
#include "init.h"
#include "space.h"

#ifndef XFS_IOC_FREE_EOFBLOCKS
#define XFS_IOC_FREE_EOFBLOCKS _IOR ('X', 58, struct xfs_eofblocks)

#define XFS_EOFBLOCKS_VERSION           1
struct xfs_fs_eofblocks {
	__u32		eof_version;
	__u32		eof_flags;
	uid_t		eof_uid;
	gid_t		eof_gid;
	prid_t		eof_prid;
	__u32		pad32;
	__u64		eof_min_file_size;
	__u64		pad64[12];
};

/* eof_flags values */
#define XFS_EOF_FLAGS_SYNC		(1 << 0) /* sync/wait mode scan */
#define XFS_EOF_FLAGS_UID		(1 << 1) /* filter by uid */
#define XFS_EOF_FLAGS_GID		(1 << 2) /* filter by gid */
#define XFS_EOF_FLAGS_PRID		(1 << 3) /* filter by project id */
#define XFS_EOF_FLAGS_MINFILESIZE	(1 << 4) /* filter by min file size */

#endif

static cmdinfo_t prealloc_cmd;

/*
 * Control preallocation amounts.
 */
static int
prealloc_f(
	int	argc,
	char	**argv)
{
	struct xfs_fs_eofblocks eofb = {0};
	int	c;

	eofb.eof_version = XFS_EOFBLOCKS_VERSION;

	while ((c = getopt(argc, argv, "g:m:p:su:")) != EOF) {
		switch (c) {
		case 'g':
			eofb.eof_flags |= XFS_EOF_FLAGS_GID;
			eofb.eof_gid = atoi(optarg);
			break;
		case 'u':
			eofb.eof_flags |= XFS_EOF_FLAGS_UID;
			eofb.eof_uid = atoi(optarg);
			break;
		case 'p':
			eofb.eof_flags |= XFS_EOF_FLAGS_PRID;
			eofb.eof_prid = atoi(optarg);
			break;
		case 's':
			eofb.eof_flags |= XFS_EOF_FLAGS_SYNC;
			break;
		case 'm':
			eofb.eof_flags |= XFS_EOF_FLAGS_MINFILESIZE;
			eofb.eof_min_file_size = cvtnum(file->geom.blocksize,
							file->geom.sectsize,
							optarg);
			break;
		case '?':
		default:
			return command_usage(&prealloc_cmd);
		}
	}
	if (optind != argc)
		return command_usage(&prealloc_cmd);

	if (xfsctl(file->name, file->fd, XFS_IOC_FREE_EOFBLOCKS, &eofb) < 0) {
		fprintf(stderr, _("%s: XFS_IOC_FREE_EOFBLOCKS on %s: %s\n"),
			progname, file->name, strerror(errno));
	}
	return 0;
}

static void
prealloc_help(void)
{
	printf(_(
"\n"
"Control speculative preallocation\n"
"\n"
"Options: [-s] [-ugp id] [-m minlen]\n"
"\n"
" -s -- synchronous flush - wait for flush to complete\n"
" -u uid -- remove prealloc on files matching user <uid>\n"
" -g gid -- remove prealloc on files matching group <gid>\n"
" -p prid -- remove prealloc on files matching project <prid>\n"
" -m minlen -- only consider files larger than <minlen>\n"
"\n"));

}

void
prealloc_init(void)
{
	prealloc_cmd.name = "prealloc";
	prealloc_cmd.altname = "prealloc";
	prealloc_cmd.cfunc = prealloc_f;
	prealloc_cmd.argmin = 1;
	prealloc_cmd.argmax = -1;
	prealloc_cmd.args = "[-s] [-ugp id] [-m minlen]\n";
	prealloc_cmd.flags = CMD_FLAG_GLOBAL;
	prealloc_cmd.oneline = _("Control specualtive preallocation");
	prealloc_cmd.help = prealloc_help;

	add_command(&prealloc_cmd);
}

