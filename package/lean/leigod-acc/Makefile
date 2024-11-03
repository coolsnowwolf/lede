#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=leigod-acc
PKG_VERSION:=1.3.0.30
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Powerful Leigod Game Accelerater
	DEPENDS:=@(x86_64||arm||mipsel||aarch64)
	URL:=https://www.leigod.com/
endef

define Package/$(PKG_NAME)/description
leigod-acc is a powerful game accelerater
endef

ifeq ($(ARCH),x86_64)
	acc-arch:=amd64
	PKG_HASH:=c26e2cde79ef21991834a94232520d1e5ac2df88a6382cb436a61865a8d9c15e
endif

ifeq ($(ARCH),mipsel)
	acc-arch:=mipsle
	PKG_HASH:=ac35cf6e19fc5028ac1da9921456726a14e0726339e89fe3e14c01a713140e57
endif

ifeq ($(ARCH),arm)
	acc-arch:=arm
	PKG_HASH:=c904bb3f41910d50ae6e8abc1e2a0b73eed6d74fb5c3b55274b82e9d4662a426
endif

ifeq ($(ARCH),aarch64)
	acc-arch:=arm64
	PKG_HASH:=316a604645f4216276dc5642d5f638a005b342771fb436a3c63826d85c5b5c1b
endif

PKG_SOURCE_URL:=http://119.3.40.126/router_plugin/
PKG_SOURCE:=acc-gw.linux.$(acc-arch)

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)/bin
	wget -O $(PKG_BUILD_DIR)/bin/acc-gw.linux.$(acc-arch) $(PKG_SOURCE_URL)$(PKG_SOURCE)
	chmod +x $(PKG_BUILD_DIR)/bin/acc-gw.linux.$(acc-arch)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/leigod
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/acc-gw.linux.$(acc-arch) $(1)/usr/sbin/leigod/acc-gw.linux.$(acc-arch)
	$(INSTALL_BIN) ./files/leigod_uninstall.sh $(1)/usr/sbin/leigod/leigod_uninstall.sh
	$(INSTALL_BIN) ./files/plugin_common.sh $(1)/usr/sbin/leigod/plugin_common.sh
	$(INSTALL_BIN) ./files/leigod-helper.sh $(1)/usr/sbin/leigod/leigod-helper.sh
	$(INSTALL_BIN) ./files/update_plugin $(1)/usr/sbin/leigod/update_plugin
	$(INSTALL_DATA) ./files/version $(1)/usr/sbin/leigod/version
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
