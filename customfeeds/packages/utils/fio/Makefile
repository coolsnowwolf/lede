#
# Copyright (C) 2018 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fio
PKG_VERSION:=3.24
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://brick.kernel.dk/snaps
PKG_HASH:=b66fe29ad0047c347e9c21090ff1b37a882fa1041bcc3a6f8501d47db15f5063

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fio
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  DEPENDS:=+libaio +zlib
  TITLE:=I/O testing and benchmarking tool
  URL:=https://github.com/axboe/fio
endef

define Package/fio/description
  Fio is an I/O testing tool that will spawn a number of threads or
  processes doing a particular type of I/O action as specified by
  the user. Fio can be driven by a 'job file' describing the I/O
  load one wants to simulate.
endef

TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS))
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_ARGS = \
	--disable-numa \
	--disable-rdma \
	--disable-rados \
	--disable-http \
	--disable-rbd \
	--disable-gfapi \
	--disable-lex \
	--disable-pmem \
	--disable-native \
	--disable-shm \
	--disable-optimizations \
	--disable-tcmalloc \
	--cc="$(TARGET_CC)" \
	--extra-cflags="$(TARGET_CFLAGS)" \
	--prefix="$(CONFIGURE_PREFIX)"

define Package/fio/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fio $(1)/usr/bin/
endef

$(eval $(call BuildPackage,fio))
