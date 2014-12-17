#
# Copyright (c) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LTLIBRARY = libxfs.la
LT_CURRENT = 0
LT_REVISION = 0
LT_AGE = 0

# headers to install in include/xfs
PKGHFILES = xfs_fs.h \
	xfs_types.h

# headers with build dependencies
SHFILES = xfs.h \
	init.h \
	crc32defs.h \
	crc32table.h

HFILES = $(addprefix support/, $(SHFILES)) \
	xfs_dir2_priv.h

# headers installed in QA targets
SQAHFILES = libxfs_io.h \
	xfs_arch.h \
	xfs_btree_trace.h \
	xfs_inode.h \
	xfs_mount.h \
	xfs_trace.h \
	xfs_trans.h

QAHFILES = $(addprefix support/, $(SQAHFILES)) \
	xfs_alloc.h \
	xfs_alloc_btree.h \
	xfs_attr_leaf.h \
	xfs_attr_sf.h \
	xfs_bit.h \
	xfs_bmap.h \
	xfs_bmap_btree.h \
	xfs_btree.h \
	xfs_attr_remote.h \
	xfs_cksum.h \
	xfs_da_btree.h \
	xfs_da_format.h \
	xfs_dir2.h \
	xfs_format.h \
	xfs_ialloc.h \
	xfs_ialloc_btree.h \
	xfs_inode_buf.h \
	xfs_inode_fork.h \
	xfs_log_format.h \
	xfs_quota_defs.h \
	xfs_sb.h \
	xfs_shared.h \
	xfs_trans_resv.h \
	xfs_trans_space.h \

# supporting C files
SCFILES = cache.c \
	crc32.c \
	init.c \
	kmem.c \
	logitem.c \
	radix-tree.c \
	rdwr.c \
	trans.c \
	util.c

CFILES = $(addprefix support/, $(SCFILES)) \
	xfs_alloc.c \
	xfs_alloc_btree.c \
	xfs_attr.c \
	xfs_attr_leaf.c \
	xfs_attr_remote.c \
	xfs_bmap.c \
	xfs_bmap_btree.c \
	xfs_btree.c \
	xfs_da_btree.c \
	xfs_da_format.c \
	xfs_dir2.c \
	xfs_dir2_block.c \
	xfs_dir2_data.c \
	xfs_dir2_leaf.c \
	xfs_dir2_node.c \
	xfs_dir2_sf.c \
	xfs_dquot_buf.c \
	xfs_ialloc.c \
	xfs_inode_buf.c \
	xfs_inode_fork.c \
	xfs_ialloc_btree.c \
	xfs_log_rlimit.c \
	xfs_rtbitmap.c \
	xfs_sb.c \
	xfs_symlink_remote.c \
	xfs_trans_resv.c

CFILES += $(addprefix support/, $(PKG_PLATFORM).c)

SPCFILES = darwin.c freebsd.c irix.c linux.c
SLSRCFILES = $(shell echo $(SPCFILES) | sed -e "s/$(PKG_PLATFORM).c//g")
SLSRCFILES += gen_crc32table.c
LSRCFILES = $(addprefix support/, $(SLSRCFILES))

#
# Tracing flags:
# -DIO_DEBUG		reads and writes of buffers
# -DMEM_DEBUG		all zone memory use
# -DLI_DEBUG		log item (ino/buf) manipulation
# -DXACT_DEBUG		transaction state changes
#
#LCFLAGS +=

FCFLAGS = -I. -Isupport

LTLIBS = $(LIBPTHREAD) $(LIBRT)

# don't try linking xfs_repair with a debug libxfs.
DEBUG = -DNDEBUG

LDIRT = $(addprefix support/, gen_crc32table crc32table.h crc32selftest)

default: xfs support/crc32selftest ltdepend $(LTLIBRARY)

# set up include/xfs header directory
xfs: $(addsuffix -hdrs, $(PKGHFILES) $(QAHFILES))

%-hdrs:
	$(Q)$(LN_S) -f $(PWD)/libxfs/$* $(TOPDIR)/include/xfs/$(notdir $*)

support/crc32table.h: support/gen_crc32table.c
	@echo "    [CC]     support/gen_crc32table"
	$(Q) $(CC) $(CFLAGS) -o support/gen_crc32table $<
	@echo "    [GENERATE] $@"
	$(Q) support/gen_crc32table > support/crc32table.h

# The selftest binary will return an error if it fails. This is made a
# dependency of the build process so that we refuse to build the tools on broken
# systems/architectures. Hence we make sure that xfsprogs will never use a
# busted CRC calculation at build time and hence avoid putting bad CRCs down on
# disk.
support/crc32selftest: $(addprefix support/, gen_crc32table.c crc32table.h crc32.c)
	@echo "    [TEST]    CRC32"
	$(Q) $(CC) $(CFLAGS) -D CRC32_SELFTEST=1 support/crc32.c -o $@
	$(Q) ./$@

include $(BUILDRULES)

install: default

install-dev: install
	$(INSTALL) -m 644 $(HFILES) $(PKG_INC_DIR)

install-qa: install-dev
	$(INSTALL) -m 644 $(QAHFILES) $(PKG_INC_DIR)

-include .ltdep

