# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=parted
PKG_VERSION:=3.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://ftp.gnu.org/gnu/parted/
PKG_HASH:=e1298022472da5589b7f2be1d5ee3c1b66ec3d96dfbad03dc642afd009da5342

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/parted
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GNU Parted
  SUBMENU:=Disc
  DEPENDS:=+libblkid +libuuid +PARTED_READLINE:libreadline +CONFIG_PARTED_READLINE:libncurses +PARTED_LVM2:libdevmapper
endef

define Package/parted/description
  GNU Parted manipulates partition tables. This is useful for
  creating space for new operating systems, reorganizing
  disk usage, copying data on hard disks and disk imaging.
endef

define Package/parted/config
  source "$(SOURCE)/Config.in"
endef

ifneq ($(CONFIG_PARTED_READLINE),y)
  CONFIGURE_ARGS += --without-readline
endif

ifneq ($(CONFIG_PARTED_LVM2),y)
  CONFIGURE_ARGS += --disable-device-mapper
endif

define Package/parted/install
	$(INSTALL_DIR) $(1)/usr/lib $(1)/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so** $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/parted $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/partprobe $(1)/sbin/
endef

$(eval $(call BuildPackage,parted))
