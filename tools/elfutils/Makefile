# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=elfutils
PKG_VERSION:=0.196
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/$(PKG_NAME)/ftp/$(PKG_VERSION)
PKG_HASH:=fd5cc6b77ad6773cac93cb3f415f9318ac3b3455eecf801f6b4a742c4f6c7209

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
	libdwfl_stacktrace \
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
	obstack-printf-posix \
	progname \
	strchrnul \
	tsearch

HOST_BUILD_PARALLEL:=1
HOST_CONFIGURE_CACHE:=1

include $(INCLUDE_DIR)/host-build.mk

export $(PKG_GNULIB_BASE)=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/.libs/$(PKG_GNULIB_BASE).a

export $(PKG_GNULIB_BASE)_exitfail=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-exitfail.o
export $(PKG_GNULIB_BASE)_fallocate-posix=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-posix_fallocate.o
export $(PKG_GNULIB_BASE)_mbszero=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-mbszero.o
export $(PKG_GNULIB_BASE)_obstack=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-obstack.o
export $(PKG_GNULIB_BASE)_obstack-printf=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/*obstack_printf.o
export $(PKG_GNULIB_BASE)_printf-args=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/*printf-args.o
export $(PKG_GNULIB_BASE)_printf-frexp=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-printf-frexp.o
export $(PKG_GNULIB_BASE)_printf-frexpl=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-printf-frexpl.o
export $(PKG_GNULIB_BASE)_printf-parse=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/*printf-parse.o
export $(PKG_GNULIB_BASE)_tsearch=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-tsearch.o
export $(PKG_GNULIB_BASE)_vasnprintf=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/*vasnprintf.o
export $(PKG_GNULIB_BASE)_xsize=$(HOST_BUILD_DIR)/$(PKG_GNULIB_BASE)/$(PKG_GNULIB_BASE)_la-xsize.o

HOST_MAKE_FLAGS += \
	SUBDIRS="$(PKG_SUBDIRS)" \
	am__CONFIG_DISTCLEAN_FILES= \
	DEFAULT_INCLUDES='$$$$(if $$$$(findstring $(lastword $(PKG_SUBDIRS)),$$$$(subdir)),,-I.) -I$$$$(top_builddir) -I$$$$(top_srcdir)/$(PKG_GNULIB_BASE)' \
	AM_LDFLAGS='$$$$(STACK_USAGE_NO_ERROR)' \
	LIBS+='$$$$(if $$$$(findstring $(lastword $(PKG_SUBDIRS)),$$$$(subdir)), $$$$($(PKG_GNULIB_BASE)))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_exitfail))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_fallocate-posix))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_mbszero))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_obstack))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_obstack-printf))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_printf-args))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_printf-frexp))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_printf-frexpl))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_printf-parse))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_tsearch))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_vasnprintf))' \
	LIBS+='$$$$(wildcard $$$$($(PKG_GNULIB_BASE)_xsize))' \
	REPLACE_FCNTL=0 REPLACE_FREE=0 REPLACE_FSTAT=0 REPLACE_OPEN=0 \
	REPLACE_OBSTACK=0 REPLACE_OBSTACK_PRINTF=0 \
	bin_PROGRAMS='$(PKG_PROGRAMS)' EXEEXT=

HOST_CPPFLAGS += "'-I$$$$(top_srcdir)/lib'"

ifeq ($(HOST_OS),Darwin)
  HOST_CFLAGS += -I/opt/homebrew/include
endif

HOST_CFLAGS += -Wno-error -fPIC -std=gnu17

HOST_CONFIGURE_ARGS += \
	--enable-maintainer-mode \
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

define Host/Configure
	$(call Host/Configure/Default)
	$(call Host/Uninstall)
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
	-$(call Host/Compile/Default,maintainer-clean) # Clean bootstrap files from the release
endef

$(eval $(call HostBuild))
