# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic
PKG_BASE_VERSION:=0.27.0-beta.9
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/UnblockNeteaseMusic/server.git
PKG_SOURCE_DATE:=2021-11-04
PKG_SOURCE_VERSION:=50d25e5189b917fcb6a053395e9b45a897b34dc9
PKG_MIRROR_HASH:=41d5a28f68312ab40eb79f502380932c0b7b896bd410e216a0ad95e219d85d6a

PKG_VERSION:=$(PKG_BASE_VERSION)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))

PKG_LICENSE:=LGPL-3.0-only
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/UnblockNeteaseMusic
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Revive Netease Cloud Music (NodeJS)
  URL:=https://github.com/nondanee/UnblockNeteaseMusic
  DEPENDS:=+node
  PKGARCH:=all
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	xzcat $(DL_DIR)/$(PKG_SOURCE) | $(TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)
endef

define Build/Configure
	echo -e $(PKG_BASE_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/core_ver
	echo -e $(PKG_SOURCE_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/local_ver
endef

define Build/Compile
endef

define Package/UnblockNeteaseMusic/install
	$(INSTALL_DIR) $(1)/usr/share/UnblockNeteaseMusic
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/* $(1)/usr/share/UnblockNeteaseMusic
endef

$(eval $(call BuildPackage,UnblockNeteaseMusic))
