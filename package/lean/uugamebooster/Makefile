#
# Copyright (C) 2021 KFERMercer <KFER.Mercer@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

#
# to get the latest version & md5 checksum:
# curl -L -s -k -H "Accept:text/plain" "http://router.uu.163.com/api/plugin?type=openwrt-$(UU_ARCH)"
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uugamebooster
PKG_VERSION:=v2.12.0
PKG_RELEASE:=9

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=@(aarch64||arm||mips||mipsel||x86_64) +kmod-tun
	TITLE:=NetEase UU Game Booster
	URL:=https://uu.163.com
endef

define Package/$(PKG_NAME)/description
NetEase's UU Game Booster Accelerates Triple-A Gameplay and Market
endef

ifeq ($(ARCH),x86_64)
	UU_ARCH:=x86_64
	PKG_MD5SUM:=61fffa3e17f21cca78c85b0f0bbcd7ed
endif

ifeq ($(ARCH),mipsel)
	UU_ARCH:=mipsel
	PKG_MD5SUM:=e72e9fa1761648113e9437f0e438f801
endif

ifeq ($(ARCH),mips)
	UU_ARCH:=mipsel
	PKG_MD5SUM:=e72e9fa1761648113e9437f0e438f801
endif

ifeq ($(ARCH),arm)
	UU_ARCH:=arm
	PKG_MD5SUM:=3c24b9c6fceab1d2b08dfd3bd027fdf5
endif

ifeq ($(ARCH),aarch64)
	UU_ARCH:=aarch64
	PKG_MD5SUM:=01be691af0a2e856be80754d49fa821a
endif

PKG_SOURCE_URL:=http://uu.gdl.netease.com/openwrt-$(UU_ARCH)/$(PKG_VERSION)/uu.tar.gz?
PKG_SOURCE:=$(PKG_NAME)-$(UU_ARCH)-$(PKG_VERSION).tar.gz

STRIP:=true

UNTAR_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-$(UU_ARCH)-bin

define Build/Prepare
	mkdir -vp $(UNTAR_DIR)
	tar -zxvf $(DL_DIR)/$(PKG_SOURCE) -C $(UNTAR_DIR)
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	# $(INSTALL_DIR) $(1)/etc/init.d
	# $(INSTALL_BIN) ./files/uugamebooster.init $(1)/etc/init.d/uuplugin

	$(INSTALL_DIR) $(1)/usr/share/$(PKG_NAME)
	$(INSTALL_BIN) $(UNTAR_DIR)/uuplugin $(1)/usr/share/$(PKG_NAME)/uuplugin
	$(INSTALL_CONF) $(UNTAR_DIR)/uu.conf $(1)/usr/share/$(PKG_NAME)/uu.conf

	# not finish yet:
	# $(INSTALL_DIR) $(1)/usr/bin
	# $(INSTALL_BIN) ./files/uugamebooster-update $(1)/usr/bin/$(PKG_NAME)
	# $(LN) $(1)/usr/bin/$(PKG_NAME)/uugamebooster-update $(1)/usr/bin/uugamebooster-update
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
