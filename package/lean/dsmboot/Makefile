#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dsmboot
PKG_VERSION:=5.2
PKG_RELEASE:=5967.1

include $(INCLUDE_DIR)/package.mk

define Package/dsmboot
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Synology DSM pxeboot from TFTP
  DEPENDS:=
endef

define Package/dsmboot/description
DSM 5.2 boot from TFTP
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/dsmboot/install
	$(INSTALL_DIR) $(1)/root
	$(INSTALL_DATA) ./files/* $(1)/root/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/dsmboot $(1)/etc/uci-defaults/yyy-dsmboot
endef

$(eval $(call BuildPackage,dsmboot))
