#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/rpcd.git
PKG_MIRROR_HASH:=18137fa6904b1a7aec03a16a4c25fd9b9689b24ee14d431bacdda9f093339071
PKG_SOURCE_DATE:=2022-09-21
PKG_SOURCE_VERSION:=8c852b656bf1622dee1ae2cfa4c083f730c1c539
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_ASLR_PIE_REGULAR:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/rpcd $(1)/usr/include/
endef

define Package/rpcd/default
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=OpenWrt ubus RPC backend server
  DEPENDS:=+libubus +libubox
endef

define Package/rpcd
  $(Package/rpcd/default)
  DEPENDS+= +libuci +libblobmsg-json +libjson-c
endef

define Package/rpcd/description
 This package provides the UBUS RPC backend server to expose various
 functionality to frontend programs via JSON-RPC.
endef

define Package/rpcd/conffiles
/etc/config/rpcd
endef

TARGET_LDFLAGS += -lcrypt

define Package/rpcd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rpcd.init $(1)/etc/init.d/rpcd
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rpcd $(1)/sbin/rpcd
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/unauthenticated.json $(1)/usr/share/rpcd/acl.d/unauthenticated.json
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/rpcd.config $(1)/etc/config/rpcd
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/50-migrate-rpcd-ubus-sock.sh $(1)/etc/uci-defaults
endef


# 1: plugin name
# 2: extra dependencies
# 3: plugin title/description
define BuildPlugin

  PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_rpcd-mod-$(1)

  define Package/rpcd-mod-$(1)
    $(Package/rpcd/default)
    TITLE+= ($(1) plugin)
    DEPENDS+=rpcd $(2)
  endef

  define Package/rpcd-mod-$(1)/description
    $(3)
  endef

  define Package/rpcd-mod-$(1)/postinst
#!/bin/sh
[ -n "$$$${IPKG_INSTROOT}" ] || /etc/init.d/rpcd reload
  endef

  define Package/rpcd-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1).so $$(1)/usr/lib/rpcd/
  endef

  $$(eval $$(call BuildPackage,rpcd-mod-$(1)))

endef

$(eval $(call BuildPackage,rpcd))
$(eval $(call BuildPlugin,file,,Provides ubus calls for file and directory operations.))
$(eval $(call BuildPlugin,rpcsys,,Provides ubus calls for sysupgrade and password changing.))
$(eval $(call BuildPlugin,iwinfo,+libiwinfo,Provides ubus calls for accessing iwinfo data.))
$(eval $(call BuildPlugin,ucode,+libucode,Allows implementing plugins using ucode scripts.))
