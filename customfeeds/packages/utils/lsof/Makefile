#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lsof
PKG_VERSION:=4.94.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).linux.tar.bz2
PKG_SOURCE_URL:=https://github.com/lsof-org/lsof/releases/download/$(PKG_VERSION)
PKG_HASH:=c41709c2543ecf9de1e950795790a9786a2f225e51c3cc53d6a9a256f872472b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION).linux

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=Unique
PKG_LICENSE_FILES:=00README

include $(INCLUDE_DIR)/package.mk

define Package/lsof
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libtirpc
  TITLE:=LiSt Open Files - a diagnostic tool
  URL:=http://people.freebsd.org/~abe/
endef

ifneq ($(CONFIG_IPV6),n)
  LINUX_CLIB_IPV6=-DHASIPv6
else
  LINUX_CLIB_IPV6=
endif

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/tirpc
TARGET_LDFLAGS += -ltirpc

define Build/Configure
	$(SED) 's/rpc\/rpc/\/tirpc\/rpc\/rpc/g' $(PKG_BUILD_DIR)/Configure
	cd $(PKG_BUILD_DIR); \
		LINUX_CLIB="-DGLIBCV=2 $(LINUX_CLIB_IPV6)" \
		LSOF_CC="$(TARGET_CC)" \
		LSOF_VSTR="$(LINUX_VERSION)" \
		LSOF_CFGF="$(TARGET_CFLAGS)" \
		LSOF_CFGL="$(TARGET_LDFLAGS)" \
		LSOF_AR="$(TARGET_CROSS)ar cr" \
		LSOF_RANLIB="$(TARGET_CROSS)ranlib" \
		LSOF_INCLUDE="$(STAGING_DIR)/usr/include" \
		./Configure -n linux
endef

define Build/Compile
	LSOF_HOST="none" \
	LSOF_LOGNAME="none" \
	LSOF_SYSINFO="none" \
	LSOF_USER="none" \
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/lsof/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lsof $(1)/usr/bin
endef

$(eval $(call BuildPackage,lsof))
