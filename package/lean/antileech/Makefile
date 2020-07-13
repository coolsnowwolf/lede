#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=antileech
PKG_VERSION:=2.3.2
PKG_RELEASE:=1

#PKG_SOURCE:=antileech-2.3.1.tar.gz
#PKG_SOURCE_URL:=@SF/amule
#PKG_MD5SUM:=a892a1cc00d03e943e03434d182fe73e

PKG_BUILD_DEPENDS:=libgd

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/antileech
  SUBMENU:=P2P
  SECTION:=net
  CATEGORY:=Network
  TITLE:=AntiLeech for amule
  URL:=http://www.amule.org/
  DEPENDS:=+libc +amule +libwxbase
endef

TARGET_LDFLAGS += \

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	sed -i "s:^AC_CHECK_PROG.WXCONFIG,.*::g" $(PKG_BUILD_DIR)/configure.ac
	sed -i "s:wx-config --cppflags:$(STAGING_DIR)/usr/bin/wx-config --cppflags:g" $(PKG_BUILD_DIR)/configure.ac
	cd $(PKG_BUILD_DIR) && sh ./autogen.sh
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(MAKE) -k -C $(PKG_BUILD_DIR) \
		HOSTCC="$(HOSTCC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Package/antileech/install
	$(INSTALL_DIR) $(1)/usr/share/amule
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/amule/libantiLeech.so $(1)/usr/share/amule
endef

$(eval $(call BuildPackage,antileech))
