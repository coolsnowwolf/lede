# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=elfutils
PKG_VERSION:=0.191
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION)
PKG_HASH:=df76db71366d1d708365fc7a6c60ca48398f14367eb2b8954efc8897147ad871

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING COPYING-GPLV2 COPYING-LGPLV3
PKG_CPE_ID:=cpe:/a:elfutils_project:elfutils

PKG_FIXUP:=autoreconf

PKG_PROGRAMS:=elflint findtextrel elfcmp unstrip stack elfcompress elfclassify srcfiles

PKG_SUBDIRS := \
	config \
	lib \
	libelf \
	libcpu \
	backends \
	libebl \
	libdwelf \
	libdwfl \
	libdw \
	src

PKG_GNULIB_BASE:=libgnu

PKG_GNULIB_ARGS = \
	--dir=$(HOST_BUILD_DIR) \
	--libtool \
	--avoid=reallocarray \
	--import

PKG_GNULIB_MODS = \
	argp \
	fallocate-posix \
	fnmatch-gnu \
	fts \
	obstack \
	progname \
	strchrnul \
	tsearch

include $(INCLUDE_DIR)/host-build.mk

export $(PKG_GNULIB_BASE)=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/.libs/$(PKG_GNULIB_BASE).a
export $(PKG_GNULIB_BASE)_fallocate-posix=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-posix_fallocate.o
export $(PKG_GNULIB_BASE)_tsearch=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-tsearch.o

HOST_MAKE_FLAGS += \
	DEFAULT_INCLUDES='-I. -I$$$$(top_builddir) -I$$$$(top_srcdir)/$(PKG_GNULIB_BASE)' \
	AM_LDFLAGS='$$$$(STACK_USAGE_NO_ERROR)' \
	LIBS+='$$$$(if $$$$(findstring $(lastword $(PKG_SUBDIRS)),$$$$(subdir)), $$$$($(PKG_GNULIB_BASE)))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_fallocate-posix))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_tsearch))' \
	REPLACE_FCNTL=0 REPLACE_FREE=0 REPLACE_FSTAT=0 REPLACE_OPEN=0 \
	bin_PROGRAMS='$(PKG_PROGRAMS)' EXEEXT=

HOST_CPPFLAGS += "'-I$$$$(top_srcdir)/lib'"

ifeq ($(HOST_OS),Darwin)
  HOST_CFLAGS += -I/opt/homebrew/include
endif

HOST_CFLAGS += -Wno-error -fPIC -std=gnu17

HOST_CONFIGURE_ARGS += \
	--without-libintl-prefix \
	--without-libiconv-prefix \
	--disable-debuginfod \
	--disable-libdebuginfod \
	--disable-nls \
	--disable-shared \
	--enable-static \
	--without-lzma \
	--without-bzlib \
	--without-zstd

ifeq ($(HOST_OS),Darwin)
  HOST_CONFIGURE_ARGS += --disable-symbol-versioning
endif

HOST_CONFIGURE_VARS += \
	ac_cv_search_argp_parse=yes \
	ac_cv_search_fts_close=yes \
	ac_cv_search__obstack_free=yes \
	ac_cv_buildid=yes

Hooks/HostConfigure/Pre := Host/Gnulib/Prepare $(Hooks/HostConfigure/Pre)
Hooks/HostCompile/Pre := Host/Gnulib/Compile $(Hooks/HostCompile/Pre)

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
