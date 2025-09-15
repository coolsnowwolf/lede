#
# Copyright (C) 2014-2015 KyleRicardo
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mentohust
PKG_VERSION:=0.3.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/KyleRicardo/MentoHUST-OpenWrt-ipk.git
PKG_SOURCE_VERSION:=557cffca8032b6d8ac948be8a79255dc64a1915d
PKG_MIRROR_HASH:=1db6cd4402ea3e1fa2adc3adcc1490fffdac224ae83024d883429b8b67fb9c74

MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk

define Package/mentohust
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A Ruijie Client Daemon
  URL:=https://github.com/KyleRicardo/MentoHUST-OpenWrt-ipk
  DEPENDS:=+libpcap
endef

define Package/mentohust/description
  A Ruijie Client Daemon, most usually used in China collages.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS) -ldl"
endef

define Package/mentohust/conffiles
/etc/mentohust.conf
endef

define Package/mentohust/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mentohust $(1)/usr/sbin/mentohust
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/src/mentohust.conf $(1)/etc/mentohust.conf
endef

$(eval $(call BuildPackage,mentohust))
