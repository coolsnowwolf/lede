# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 KFERMercer <KFER.Mercer@gmail.com>

# to get the latest version & md5 checksum:
# curl -L -s -k -H "Accept:text/plain" "https://router.uu.163.com/api/plugin?type=openwrt-$(ARCH)"

include $(TOPDIR)/rules.mk

PKG_NAME:=uugamebooster
PKG_VERSION:=2.17.8
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(ARCH)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://uu.gdl.netease.com/uuplugin/openwrt-$(ARCH)/v$(PKG_VERSION)/uu.tar.gz?
ifeq ($(ARCH),aarch64)
  PKG_MD5SUM:=ebfe9e59227ba79e4a6c9acd5e52726d
else ifeq ($(ARCH),arm)
  PKG_MD5SUM:=7634863f8f545b1eab3fec2d9af1bc39
else ifeq ($(ARCH),mipsel)
  PKG_MD5SUM:=de83157187dca30ea00d0924bd7edd3d
else ifeq ($(ARCH),x86_64)
  PKG_MD5SUM:=232169ae847991e98dd73b8a873aa751
endif

include $(INCLUDE_DIR)/package.mk

STRIP:=true

TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

define Package/uugamebooster
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@(aarch64||arm||mipsel||x86_64) +kmod-tun
  TITLE:=NetEase UU Game Booster
  URL:=https://uu.163.com
endef

define Package/uugamebooster/description
  NetEase's UU Game Booster Accelerates Triple-A Gameplay and Market.
endef

define Build/Compile
endef

define Package/uugamebooster/conffiles
/usr/share/uugamebooster/uu.conf
endef

define Package/uugamebooster/install
	# $(INSTALL_DIR) $(1)/etc/init.d
	# $(INSTALL_BIN) ./files/uugamebooster.init $(1)/etc/init.d/uugamebooster

	$(INSTALL_DIR) $(1)/usr/share/uugamebooster
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uuplugin $(1)/usr/share/uugamebooster/uuplugin
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/uu.conf $(1)/usr/share/uugamebooster/uu.conf

	# not finish yet:
	# $(INSTALL_DIR) $(1)/usr/bin
	# $(INSTALL_BIN) ./files/uugamebooster-update $(1)/usr/bin/uugamebooster
	# $(LN) ./uugamebooster/uugamebooster-update $(1)/usr/bin/uugamebooster-update
endef

$(eval $(call BuildPackage,uugamebooster))
