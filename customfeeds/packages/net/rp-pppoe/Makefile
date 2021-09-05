#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rp-pppoe
PKG_VERSION:=3.14
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://dianne.skoll.ca/projects/rp-pppoe/download
PKG_HASH:=7825232f64ab4d618ef074d62d145ae43d6edc91b9a718c6130a4742bac40e2a

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.0-or-later
PKG_LICENSE_FILES:=doc/LICENSE
PKG_CPE_ID:=cpe:/a:roaring_penguin:pppoe

PKG_BUILD_DEPENDS:=ppp
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/rp-pppoe/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=PPPoE (PPP over Ethernet)
  URL:=https://dianne.skoll.ca/projects/rp-pppoe/
  SUBMENU:=Dial-in/up
endef

define Package/rp-pppoe/Default/description
  Dubbed RP-PPPoE, this open-source product is ideal for Linux users with
  a DSL "modem" whose Internet service provider uses PPPoE.
endef

define Package/rp-pppoe-common
  $(call Package/rp-pppoe/Default)
  TITLE+= common
  DEPENDS:=ppp
endef

define Package/rp-pppoe-common/description
$(call Package/rp-pppoe/Default/description)

 This package contains files common to server and relay.
endef

define Package/rp-pppoe-common/conffiles
/etc/config/pppoe
endef

define Package/rp-pppoe-relay
  $(call Package/rp-pppoe/Default)
  DEPENDS:=+rp-pppoe-common
  TITLE+= relay
  DEFAULT:=n
endef

define Package/rp-pppoe-relay/description
$(call Package/rp-pppoe/Default/description)

 This package contains pppoe-relay.
endef

define Package/rp-pppoe-relay/conffiles
/etc/default/pppoe-relay
endef

define Package/rp-pppoe-server
  $(call Package/rp-pppoe/Default)
  DEPENDS:=+rp-pppoe-common +ppp-mod-pppoe
  TITLE+= server
endef

define Package/rp-pppoe-server/conffiles
/etc/ppp/pppoe-server-options
/etc/ppp/options
/etc/ppp/pap-secrets
/etc/ppp/chap-secrets
/etc/default/pppoe-server
endef

define Package/rp-pppoe-server/description
$(call Package/rp-pppoe/Default/description)

 This package contains pppoe-server.
endef

define Package/rp-pppoe-sniff
  $(call Package/rp-pppoe/Default)
  DEPENDS:=+ppp @BROKEN
  TITLE+= sniffer
endef

define Package/rp-pppoe-sniff/description
$(call Package/rp-pppoe/Default/description)

 This package contains pppoe-sniff.
endef

TARGET_CFLAGS += -I$(PKG_BUILD_DIR)/src/libevent -isystem $(PKG_BUILD_DIR)/missing-headers -D_BSD_SOURCE
CONFIGURE_PATH := ./src
CONFIGURE_ARGS += ac_cv_path_PPPD=/usr/sbin/pppd --enable-plugin=$(STAGING_DIR)/usr/include/
MAKE_FLAGS := DESTDIR="$(PKG_INSTALL_DIR)" PLUGIN_PATH=pppoe.so install
MAKE_PATH := ./src

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/missing-headers/net
	$(CP) ./files/ppp_defs.h $(PKG_BUILD_DIR)/missing-headers/net/
	$(call Build/Prepare/Default,$(1),$(2),$(3))
endef

define Package/rp-pppoe-common/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/pppoe.config $(1)/etc/config/pppoe
endef

define Package/rp-pppoe-relay/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/pppoe-relay.init $(1)/etc/init.d/pppoe-relay
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/pppoe-relay $(1)/usr/sbin/
	$(INSTALL_DATA) ./files/pppoe-relay.default $(1)/etc/uci-defaults/
endef

define Package/rp-pppoe-server/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/pppoe-server.init $(1)/etc/init.d/pppoe-server
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_CONF) ./files/ppp/{options,pap-secrets,pppoe-server-options} $(1)/etc/ppp/
	$(INSTALL_DIR) $(1)/etc/ppp/plugins
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/pppoe-server $(1)/usr/sbin/
	$(INSTALL_DATA) ./files/pppoe-server.default $(1)/etc/uci-defaults/
endef

define Package/rp-pppoe-sniff/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/pppoe-sniff $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,rp-pppoe-common))
$(eval $(call BuildPackage,rp-pppoe-relay))
$(eval $(call BuildPackage,rp-pppoe-server))
$(eval $(call BuildPackage,rp-pppoe-sniff))
