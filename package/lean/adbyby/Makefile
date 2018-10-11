#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=adbyby
PKG_VERSION:=2.7
PKG_RELEASE:=20181008

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Powerful adblock module to block ad.
	DEPENDS:=
	URL:=http://www.adbyby.com/
endef

define Package/$(PKG_NAME)/description
Adbyby is a powerful adblock module to block ad,just like adblock.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/adbyby
	$(INSTALL_BIN) ./files/adbyby.sh $(1)/usr/share/adbyby/
	$(INSTALL_BIN) ./files/adbybyfirst.sh $(1)/usr/share/adbyby/
	$(INSTALL_BIN) ./files/adbybyupdate.sh $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/adhook.ini $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/user.action $(1)/usr/share/adbyby/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/data
	$(INSTALL_DATA) ./files/data/* $(1)/usr/share/adbyby/data/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/doc
	$(INSTALL_DATA) ./files/doc/* $(1)/usr/share/adbyby/doc/

ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ./files/7620n/adbyby $(1)/usr/share/adbyby/
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ./files/ar71xx/adbyby $(1)/usr/share/adbyby/
endif
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/x86/adbyby $(1)/usr/share/adbyby/
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/x86_64/adbyby $(1)/usr/share/adbyby/
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/arm/adbyby $(1)/usr/share/adbyby/
endif
ifeq ($(ARCH),aarch64)
	$(INSTALL_BIN) ./files/armv7/adbyby $(1)/usr/share/adbyby/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
