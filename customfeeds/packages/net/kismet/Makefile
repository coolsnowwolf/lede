#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kismet
PKG_VERSION:=2016-07-R1
PKG_RELEASE:=6

PKG_LICENSE:=LGPLv2.1
PKG_CPE_ID:=cpe:/a:kismet:kismet

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://www.kismetwireless.net/code
PKG_HASH:=bdb21f153311f1ff3b16621bf0d6740f66369bf0982b0a289c9a12af8847e237

include $(INCLUDE_DIR)/package.mk

define Package/kismet/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Kismet
  MAINTAINER:=Jean-Michel lacroix <lacroix@lepine-lacroix.info>
  DEPENDS:= +libstdcpp +libnl @BROKEN
  URL:=http://www.kismetwireless.net/
  SUBMENU:=Wireless
endef

define Package/kismet/Default/description
 An 802.11 layer2 wireless network detector, sniffer, and intrusion
 detection system.
endef

define Package/kismet-client/conffiles
/etc/kismet/kismet.conf
endef

define Package/kismet-drone/conffiles
/etc/config/kismet_drone
/etc/kismet/kismet_drone.conf
endef

define Package/kismet-server/conffiles
/etc/config/kismet_server
/etc/kismet/kismet.conf
endef

define Package/kismet-client
$(call Package/kismet/Default)
  TITLE+= client
  DEPENDS+= +libncurses
endef

define Package/kismet-client/description
$(call Package/kismet/Default/description)
 This package contains the kismet text interface client.
endef

define Package/kismet-drone
$(call Package/kismet/Default)
  DEPENDS+= +libpcap +libpcre +libcap +wireless-tools 
  TITLE+= drone
endef

define Package/kismet-drone/description
$(call Package/kismet/Default/description)
 This package contains the kismet remote sniffing.and monitoring drone.
endef

define Package/kismet-server
$(call Package/kismet/Default)
  DEPENDS+= +libpcap +libpcre +libcap +wireless-tools
  TITLE+= server
endef

define Package/kismet-server/description
$(call Package/kismet/Default/description)
 This package contains the kismet server.
endef

CONFIGURE_ARGS += \
	--sysconfdir=/etc/kismet \

CONFIGURE_VARS += \
	CXXFLAGS="$$$$CXXFLAGS -fno-rtti"  \
	LIBS="-lm" \
	CLIENTCLIBS="-lm"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		LD="$(TARGET_CXX)" \
		all
endef

define Package/kismet/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/kismet $(1)/usr/bin/kismet
endef

define Package/kismet-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kismet_client $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/kismet/
	$(INSTALL_CONF) ./files/kismet.conf $(1)/etc/kismet/
endef

define Package/kismet-drone/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kismet_drone $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/kismet/
	$(INSTALL_CONF) ./files/kismet_drone.conf $(1)/etc/kismet/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/kismet_drone.config $(1)/etc/config/kismet_drone

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/kismet_drone.init $(1)/etc/init.d/kismet_drone
endef

define Package/kismet-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kismet_server $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/kismet/
	$(INSTALL_CONF) ./files/kismet.conf $(1)/etc/kismet/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/kismet_server.config $(1)/etc/config/kismet_server

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/kismet_server.init $(1)/etc/init.d/kismet_server
endef

$(eval $(call BuildPackage,kismet-client))
$(eval $(call BuildPackage,kismet-drone))
$(eval $(call BuildPackage,kismet-server))
