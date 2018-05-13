include $(TOPDIR)/rules.mk

PKG_NAME:=apfree_wifidog
PKG_VERSION:=v2.10.1437
PKG_RELEASE=1


PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Dengfeng Liu<liudengfeng@kunteng.org>
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://gitee.com/apfree/apfree_wifidog.git
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=dc1a052bd61c1224dda9548475c1feda9332c40fc2190ca6e4901d432379b51c

PKG_INSTALL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+zlib +iptables-mod-extra +iptables-mod-ipopt +kmod-ipt-nat +iptables-mod-nat-extra \
           +libpthread +libopenssl +@OPENSSL_WITH_EC +@OPENSSL_WITH_DEPRECATED +@OPENSSL_WITH_PSK +libjson-c +ipset +libip4tc +libevent2 +libevent2-openssl \
		   +fping +libmosquitto +libuci
  TITLE:=Apfree's wireless captive portal solution
  URL:=http://www.kunteng.org
endef

define Package/$(PKG_NAME)/description
	The ApFree Wifidog project is a complete and embeddable captive
	portal solution for wireless community groups or individuals
	who wish to open a free Hotspot while still preventing abuse
	of their Internet connection.
    It's enhanced wifidog
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifidog $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wdctl $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhttpd.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/wdping $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-msg.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html.front $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html.rear $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/authserver-offline.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/internet-offline.html $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifidog.init $(1)/etc/init.d/wifidog
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/wifidog.conf $(1)/etc/config/wifidog
	$(CP) ./files/apfree.* $(1)/etc/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))