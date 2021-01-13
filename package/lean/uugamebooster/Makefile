#
# Copyright (C) 2021 KFERMercer <KFER.Mercer@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

# 
# to get the latest version & md5 checksum:
# curl -L -s -k -H "Accept:text/plain" "http://router.uu.163.com/api/plugin?type=openwrt-$(PKG_ARCH_UU)"
# 

include $(TOPDIR)/rules.mk

PKG_NAME:=uugamebooster
DEPENDS:=@(aarch64||arm||mips||mipsel||x86_64)
PKG_VERSION:=v2.7.2
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=NetEase UU Game Booster
	URL:=https://uu.163.com
endef


define Package/$(PKG_NAME)/description
NetEase's UU Game Booster Accelerates Triple-A Gameplay and Market
endef

STRIP:=true

ifeq ($(ARCH),x86_64)
	PKG_ARCH_UU:=x86_64
	PKG_MD5SUM:=07bf06131528276e7d4b8e3992fa2e8d
endif

ifeq ($(ARCH),mipsel)
	PKG_ARCH_UU:=mipsel
	PKG_MD5SUM:=3f11af92eb7abf2af2cce8102fea7499
endif

ifeq ($(ARCH),mips)
	PKG_ARCH_UU:=mipsel
	PKG_MD5SUM:=3f11af92eb7abf2af2cce8102fea7499
endif

ifeq ($(ARCH),arm)
	PKG_ARCH_UU:=arm
	PKG_MD5SUM:=5fce2b60df66ea8a8db4ddd55da3c6b4
endif

ifeq ($(ARCH),aarch64)
	PKG_ARCH_UU:=aarch64
	PKG_MD5SUM:=bc0e513cfce113c69b068e198e936100
endif

PKG_SOURCE:=uu.tar.gz
PKG_SOURCE_URL:=http://uu.gdl.netease.com/openwrt-$(PKG_ARCH_UU)/$(PKG_VERSION)/

UNZIP_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-unzip

define Build/Prepare
	mkdir -vp $(UNZIP_DIR)
	tar -zxvf $(DL_DIR)/$(PKG_SOURCE) -C $(UNZIP_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/uugamebooster.init $(1)/etc/init.d/uugamebooster

	$(INSTALL_DIR) $(1)/usr/share/$(PKG_NAME)
	$(INSTALL_BIN) $(UNZIP_DIR)/uuplugin $(1)/usr/share/$(PKG_NAME)/uuplugin
	$(INSTALL_CONF) $(UNZIP_DIR)/uu.conf $(1)/usr/share/$(PKG_NAME)/uu.conf

	# not finish yet:
	# $(INSTALL_DIR) $(1)/usr/bin
	# $(INSTALL_BIN) ./files/uugamebooster-update $(1)/usr/bin/$(PKG_NAME)
	# $(LN) $(1)/usr/bin/$(PKG_NAME)/uugamebooster-update $(1)/usr/bin/uugamebooster-update
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
