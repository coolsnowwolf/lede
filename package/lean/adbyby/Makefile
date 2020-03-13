#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=adbyby
PKG_VERSION:=2.7
PKG_RELEASE:=20200310

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Powerful adblock module to block ad.
	DEPENDS:=@(x86||x86_64||arm||mipsel||mips||aarch64)
	URL:=http://www.adbyby.com/
endef

define Package/$(PKG_NAME)/description
Adbyby is a powerful adblock module to block ad,just like adblock.
endef

ifeq ($(ARCH),x86_64)
	ADBYBY_DIR:=amd64
endif
ifeq ($(ARCH),mipsel)
	ADBYBY_DIR:=mipsle
endif
ifeq ($(ARCH),mips)
	ADBYBY_DIR:=mips
endif
ifeq ($(ARCH),i386)
	ADBYBY_DIR:=x86
endif
ifeq ($(ARCH),arm)
	ADBYBY_DIR:=armv7
	ifeq ($(BOARD),bcm53xx)
		ADBYBY_DIR:=arm
  endif
  ifeq ($(BOARD),kirkwood)
		ADBYBY_DIR:=arm
  endif
endif
ifeq ($(ARCH),aarch64)
	ADBYBY_DIR:=armv7
endif

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/adbyby
	$(INSTALL_BIN) ./files/adbyby.sh $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/adhook.ini $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/user.action $(1)/usr/share/adbyby/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/data
	$(INSTALL_DATA) ./files/data/* $(1)/usr/share/adbyby/data/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/doc
	$(INSTALL_DATA) ./files/doc/* $(1)/usr/share/adbyby/doc/

	$(INSTALL_BIN) ./files/$(ADBYBY_DIR)/adbyby $(1)/usr/share/adbyby/adbyby
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
