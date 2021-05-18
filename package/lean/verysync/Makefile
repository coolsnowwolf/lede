#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ifeq ($(ARCH),x86_64)
	PKG_ARCH_VERYSYNC:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_VERYSYNC:=mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_VERYSYNC:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_VERYSYNC:=386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_VERYSYNC:=arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_VERYSYNC:=arm64
endif
ifeq ($(ARCH),powerpc64)
	PKG_ARCH_VERYSYNC:=ppc64
endif

PKG_NAME:=verysync
PKG_VERSION:=2.6.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-linux-$(PKG_ARCH_VERYSYNC)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dl.verysync.com/releases/v$(PKG_VERSION)/
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

TAR_CMD:=$(HOST_TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)

define Package/verysync
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A efficient data transmission tool
  URL:=http://www.verysync.com
endef

define Build/Compile
endef

define Package/verysync/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/verysync-linux-$(PKG_ARCH_VERYSYNC)-$(PKG_VERSION)/verysync $(1)/usr/bin/verysync
endef

$(eval $(call BuildPackage,verysync))
