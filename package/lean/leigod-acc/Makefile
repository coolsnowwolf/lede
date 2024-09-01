#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=leigod-acc
PKG_VERSION:=1.3.0.30
PKG_RELEASE:=1

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
endif
ifeq ($(ARCH),mipsel)
	acc-arch:=mipsle
endif
ifeq ($(ARCH),arm)
	acc-arch:=arm
endif
ifeq ($(ARCH),aarch64)
	acc-arch:=arm64
endif

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/leigod
	$(INSTALL_BIN) ./files/acc-gw.linux.$(acc-arch) $(1)/usr/sbin/leigod/acc-gw.linux.$(acc-arch)
	$(INSTALL_BIN) ./files/leigod_uninstall.sh $(1)/usr/sbin/leigod/leigod_uninstall.sh
	$(INSTALL_BIN) ./files/plugin_common.sh $(1)/usr/sbin/leigod/plugin_common.sh
	$(INSTALL_BIN) ./files/update_plugin $(1)/usr/sbin/leigod/update_plugin
	$(INSTALL_DATA) ./files/version $(1)/usr/sbin/leigod/version
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
