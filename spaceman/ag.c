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
#include <linux/dqblk_xfs.h>
#include "input.h"
#include "init.h"
#include "space.h"

#ifndef XFS_IOC_AGCONTROL
#define XFS_IOC_AGCONTROL _IOWR ('X', 60, struct xfs_agcontrol)

#define XFS_AGCONTROL_VERSION		1
struct xfs_agcontrol {
	__u32		version;
	__u32		flags;
	__u32		agno;
	__u32		state;
	__u64		pad[8];
};

/* control flags */
#define XFS_AGCONTROL_GETAGFSTATE	(1 << 0)	/* get AGF state */
#define XFS_AGCONTROL_SETAGFSTATE	(1 << 1)	/* set AGF state */
#define XFS_AGCONTROL_GETAGISTATE	(1 << 2)	/* get AGI state */
#define XFS_AGCONTROL_SETAGISTATE	(1 << 3)	/* set AGI state */

/* state flags */

/*
 * inode and allocation states are split. AGF and AGI online state will move in
 * sync as it is really a whole AG state. No allocation flags imply no new
 * allocations, but inodes and extents can be removed. Readonly means no
 * modification (alloc or free) is allowed. This is to allow different
 * operations to be performed. e.g. emptying an AG in preparation for a shrink
 * require NOALLOC state, but an AG that has a corrupted freespace btree might
 * be switched to READONLY until the freespace tree is rebuilt. An AGF/AGI in
 * this corrupt/ro state will set the relevant corruption flag in the state
 * field....
 */
#define XFS_AGFSTATE_ONLINE		(1 << 0)	/* AGF online */
#define XFS_AGFSTATE_NOALLOC		(1 << 1)	/* No new allocation */
#define XFS_AGFSTATE_READONLY		(1 << 2)	/* AGF is immutable */
#define XFS_AGFSTATE_METADATA		(1 << 3)	/* metadata preferred */
#define XFS_AGFSTATE_CORRUPT_BNO	(1 << 4)	/* bno freespace corrupt */
#define XFS_AGFSTATE_CORRUPT_CNT	(1 << 5)	/* cnt freespace corrupt */
#define XFS_AGFSTATE_CORRUPT_AGFL	(1 << 6)	/* AGFL freespace corrupt */

#define XFS_AGISTATE_ONLINE		(1 << 0)	/* AGI online */
#define XFS_AGISTATE_NOALLOC		(1 << 1)	/* No new allocation */
#define XFS_AGISTATE_READONLY		(1 << 2)	/* AGI is immutable */
#define XFS_AGISTATE_CORRUPT_TREE	(1 << 2)	/* AGI btree corrupt */

#endif

static cmdinfo_t agfctl_cmd;
static cmdinfo_t agictl_cmd;

static int
agfctl_f(
	int		argc,
	char		**argv)
{
	struct xfs_agcontrol agctl = {0};
	xfs_agnumber_t	agno;
	int		gflag = 0;
	int		c;

	while ((c = getopt(argc, argv, "gs")) != EOF) {
		switch (c) {
		case 'g':
			gflag = 1;
			break;
		default:
			return command_usage(&agfctl_cmd);
		}
	}
	if (optind != argc - 1)
		return command_usage(&agfctl_cmd);

	agno = atoi(argv[optind]);
	if (agno >= file->geom.agcount) {
		fprintf(stderr, _("%s: agno %d out of range (max %d)\n"),
			progname, agno, file->geom.agcount);
		exitcode = 1;
		return 0;
	}

	agctl.version = XFS_AGCONTROL_VERSION;
	agctl.agno = agno;
	if (gflag)
		agctl.flags = XFS_AGCONTROL_GETAGFSTATE;

	if (xfsctl(file->name, file->fd, XFS_IOC_AGCONTROL, &agctl) < 0) {
		fprintf(stderr, _("%s: XFS_IOC_AGCONTROL on %s: %s\n"),
			progname, file->name, strerror(errno));
	}
	return 0;
}

static void
agfctl_help(void)
{
	printf(_(
"\n"
"AGF state control\n"
"\n"
"Options: [-g] agno\n"
"\n"
" -g -- get state\n"
" agno -- AG to operate on\n"
"\n"));

}

void
agfctl_init(void)
{
	agfctl_cmd.name = "agfctl";
	agfctl_cmd.altname = "agfctl";
	agfctl_cmd.cfunc = agfctl_f;
	agfctl_cmd.argmin = 2;
	agfctl_cmd.argmax = -1;
	agfctl_cmd.args = "agno\n";
	agfctl_cmd.flags = CMD_FLAG_GLOBAL;
	agfctl_cmd.oneline = _("AGF state control");
	agfctl_cmd.help = agfctl_help;

	add_command(&agfctl_cmd);
}

static int
agictl_f(
	int		argc,
	char		**argv)
{
	struct xfs_agcontrol agctl = {0};
	xfs_agnumber_t	agno;
	int		gflag = 0;
	int		c;

	while ((c = getopt(argc, argv, "gs")) != EOF) {
		switch (c) {
		case 'g':
			gflag = 1;
			break;
		default:
			return command_usage(&agictl_cmd);
		}
	}
	if (optind != argc - 1)
		return command_usage(&agictl_cmd);

	agno = atoi(argv[optind]);
	if (agno >= file->geom.agcount) {
		fprintf(stderr, _("%s: agno %d out of range (max %d)\n"),
			progname, agno, file->geom.agcount);
		exitcode = 1;
		return 0;
	}

	agctl.version = XFS_AGCONTROL_VERSION;
	agctl.agno = agno;
	if (gflag)
		agctl.flags = XFS_AGCONTROL_GETAGISTATE;

	if (xfsctl(file->name, file->fd, XFS_IOC_AGCONTROL, &agctl) < 0) {
		fprintf(stderr, _("%s: XFS_IOC_AGCONTROL on %s: %s\n"),
			progname, file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}
	return 0;
}

static void
agictl_help(void)
{
	printf(_(
"\n"
"AGI state control\n"
"\n"
"Options: [-g] agno\n"
"\n"
" -g -- get state\n"
" agno -- AG to operate on\n"
"\n"));

}

void
agictl_init(void)
{
	agictl_cmd.name = "agictl";
	agictl_cmd.altname = "agictl";
	agictl_cmd.cfunc = agictl_f;
	agictl_cmd.argmin = 2;
	agictl_cmd.argmax = -1;
	agictl_cmd.args = "agno\n";
	agictl_cmd.flags = CMD_FLAG_GLOBAL;
	agictl_cmd.oneline = _("AGI state control");
	agictl_cmd.help = agictl_help;

	add_command(&agictl_cmd);
}
