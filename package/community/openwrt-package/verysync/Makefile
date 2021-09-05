#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
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
PKG_VERSION:=v2.8.2
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-linux-$(PKG_ARCH_VERYSYNC)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dl.verysync.com/releases/$(PKG_VERSION)/
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A efficient data transmission tool
	DEPENDS:=
	URL:=http://www.verysync.com
endef

define Package/$(PKG_NAME)/description
  Verysync is a efficient data transmission tool.
endef

define Build/Prepare
	tar -xzvf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME)-linux-$(PKG_ARCH_VERYSYNC)-$(PKG_VERSION)/verysync $(1)/usr/bin/verysync
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
