# Rules.make for busybox
#
# Copyright (C) 1999-2005 by Erik Andersen <andersen@codepoet.org>
#
# Licensed under GPLv2, see the file LICENSE in this tarball for details.
#

# Pull in the user's busybox configuration
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(top_builddir)/.config
endif

#--------------------------------------------------------
PROG      := busybox
MAJOR_VERSION   :=1
MINOR_VERSION   :=1
SUBLEVEL_VERSION:=0
EXTRAVERSION    :=
VERSION   :=$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL_VERSION)$(EXTRAVERSION)
BUILDTIME := $(shell TZ=UTC date -u "+%Y.%m.%d-%H:%M%z")


#--------------------------------------------------------
# With a modern GNU make(1) (highly recommended, that's what all the
# developers use), all of the following configuration values can be
# overridden at the command line.  For example:
#   make CROSS=powerpc-linux- top_srcdir="$HOME/busybox" PREFIX=/mnt/app
#--------------------------------------------------------

# If you are running a cross compiler, you will want to set 'CROSS'
# to something more interesting...  Target architecture is determined
# by asking the CC compiler what arch it compiles things for, so unless
# your compiler is broken, you should not need to specify TARGET_ARCH
CROSS           =$(subst ",, $(strip $(CROSS_COMPILER_PREFIX)))
CC             = $(CROSS)gcc
AR             = $(CROSS)ar
AS             = $(CROSS)as
LD             = $(CROSS)ld
NM             = $(CROSS)nm
STRIP          = $(CROSS)strip
CPP            = $(CC) -E
# MAKEFILES      = $(top_builddir)/.config
RM             = rm
RM_F           = $(RM) -f
LN             = ln
LN_S           = $(LN) -s
MKDIR          = mkdir
MKDIR_P        = $(MKDIR) -p
MV             = mv
CP             = cp


# What OS are you compiling busybox for?  This allows you to include
# OS specific things, syscall overrides, etc.
TARGET_OS=linux

# Select the compiler needed to build binaries for your development system
HOSTCC    = gcc
HOSTCFLAGS= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer

# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL:= C

# If you want to add some simple compiler switches (like -march=i686),
# especially from the command line, use this instead of CFLAGS directly.
# For optimization overrides, it's better still to set OPTIMIZATION.
CFLAGS_EXTRA=$(subst ",, $(strip $(EXTRA_CFLAGS_OPTIONS)))

# To compile vs some other alternative libc, you may need to use/adjust
# the following lines to meet your needs...
#
# If you are using Red Hat 6.x with the compatible RPMs (for developing under
# Red Hat 5.x and glibc 2.0) uncomment the following.  Be sure to read about
# using the compatible RPMs (compat-*) at http://www.redhat.com !
#LIBCDIR:=/usr/i386-glibc20-linux
#
# For other libraries, you are on your own.  But these may (or may not) help...
#LDFLAGS+=-nostdlib
#LIBRARIES:=$(LIBCDIR)/lib/libc.a -lgcc
#CROSS_CFLAGS+=-nostdinc -I$(LIBCDIR)/include -I$(GCCINCDIR) -funsigned-char
#GCCINCDIR:=$(shell gcc -print-search-dirs | sed -ne "s/install: \(.*\)/\1include/gp")

WARNINGS=-Wall -Wstrict-prototypes -Wshadow
CFLAGS=-I$(top_builddir)/include -I$(top_srcdir)/include -I$(srcdir)
ARFLAGS=cru


# gcc centric. Perhaps fiddle with findstring gcc,$(CC) for the rest
# get the CC MAJOR/MINOR version
CC_MAJOR:=$(shell printf "%02d" $(shell echo __GNUC__ | $(CC) -E -xc - | tail -n 1))
CC_MINOR:=$(shell printf "%02d" $(shell echo __GNUC_MINOR__ | $(CC) -E -xc - | tail -n 1))

#--------------------------------------------------------
export VERSION BUILDTIME HOSTCC HOSTCFLAGS CROSS CC AR AS LD NM STRIP CPP
ifeq ($(strip $(TARGET_ARCH)),)
TARGET_ARCH:=$(shell $(CC) -dumpmachine | sed -e s'/-.*//' \
		-e 's/i.86/i386/' \
		-e 's/sparc.*/sparc/' \
		-e 's/arm.*/arm/g' \
		-e 's/m68k.*/m68k/' \
		-e 's/ppc/powerpc/g' \
		-e 's/v850.*/v850/g' \
		-e 's/sh[234]/sh/' \
		-e 's/mips-.*/mips/' \
		-e 's/mipsel-.*/mipsel/' \
		-e 's/cris.*/cris/' \
		)
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if [ "$(1)" != "" ]; then \
		if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
		then echo "$(1)"; else echo "$(2)"; fi \
	fi)

# Setup some shortcuts so that silent mode is silent like it should be
ifeq ($(subst s,,$(MAKEFLAGS)),$(MAKEFLAGS))
export MAKE_IS_SILENT=n
SECHO=@echo
else
export MAKE_IS_SILENT=y
SECHO=-@false
endif

CFLAGS+=$(call check_gcc,-funsigned-char,)

#--------------------------------------------------------
# Arch specific compiler optimization stuff should go here.
# Unless you want to override the defaults, do not set anything
# for OPTIMIZATION...

# use '-Os' optimization if available, else use -O2
OPTIMIZATION:=$(call check_gcc,-Os,-O2)

# Some nice architecture specific optimizations
ifeq ($(strip $(TARGET_ARCH)),arm)
	OPTIMIZATION+=-fstrict-aliasing
endif
ifeq ($(strip $(TARGET_ARCH)),i386)
	OPTIMIZATION+=$(call check_gcc,-march=i386,)
	OPTIMIZATION+=$(call check_gcc,-mpreferred-stack-boundary=2,)
	OPTIMIZATION+=$(call check_gcc,-falign-functions=0 -falign-jumps=0 -falign-loops=0,\
		-malign-functions=0 -malign-jumps=0 -malign-loops=0)
endif
OPTIMIZATIONS:=$(OPTIMIZATION) -fomit-frame-pointer

#
#--------------------------------------------------------
# If you're going to do a lot of builds with a non-vanilla configuration,
# it makes sense to adjust parameters above, so you can type "make"
# by itself, instead of following it by the same half-dozen overrides
# every time.  The stuff below, on the other hand, is probably less
# prone to casual user adjustment.
#

ifeq ($(strip $(CONFIG_LFS)),y)
    # For large file summit support
    CFLAGS+=-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
endif
ifeq ($(strip $(CONFIG_DMALLOC)),y)
    # For testing mem leaks with dmalloc
    CFLAGS+=-DDMALLOC
    LIBRARIES:=-ldmalloc
else
    ifeq ($(strip $(CONFIG_EFENCE)),y)
	LIBRARIES:=-lefence
    endif
endif
ifeq ($(strip $(CONFIG_DEBUG)),y)
    CFLAGS  +=$(WARNINGS) -g -D_GNU_SOURCE
    LDFLAGS +=-Wl,-warn-common
    STRIPCMD:=/bin/true -Not_stripping_since_we_are_debugging
else
    CFLAGS+=$(WARNINGS) $(OPTIMIZATIONS) -D_GNU_SOURCE -DNDEBUG
    LDFLAGS += -Wl,-warn-common
    STRIPCMD:=$(STRIP) -s --remove-section=.note --remove-section=.comment
endif
ifeq ($(strip $(CONFIG_STATIC)),y)
    LDFLAGS += --static
endif

ifeq ($(strip $(CONFIG_SELINUX)),y)
    LIBRARIES += -lselinux
endif

ifeq ($(strip $(PREFIX)),)
    PREFIX:=`pwd`/_install
endif

# Additional complications due to support for pristine source dir.
# Include files in the build directory should take precedence over
# the copy in top_srcdir, both during the compilation phase and the
# shell script that finds the list of object files.
# Work in progress by <ldoolitt@recycle.lbl.gov>.


OBJECTS:=$(APPLET_SOURCES:.c=.o) busybox.o usage.o applets.o
CFLAGS    += $(CROSS_CFLAGS)
ifdef BB_INIT_SCRIPT
    CFLAGS += -DINIT_SCRIPT='"$(BB_INIT_SCRIPT)"'
endif

# Put user-supplied flags at the end, where they
# have a chance of winning.
CFLAGS += $(CFLAGS_EXTRA)

#------------------------------------------------------------
# Installation options
ifeq ($(strip $(CONFIG_INSTALL_APPLET_HARDLINKS)),y)
INSTALL_OPTS=--hardlinks
endif
ifeq ($(strip $(CONFIG_INSTALL_APPLET_SYMLINKS)),y)
INSTALL_OPTS=--symlinks
endif
ifeq ($(strip $(CONFIG_INSTALL_APPLET_DONT)),y)
INSTALL_OPTS=
endif

.PHONY: dummy
