#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=comgt
PKG_VERSION:=0.32
PKG_RELEASE:=30

PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).tgz
PKG_SOURCE_URL:=@SF/comgt
PKG_HASH:=0cedb2a5aa608510da66a99aab74df3db363df495032e57e791a2ff55f1d7913

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0+

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME).$(PKG_VERSION)
PKG_CHECK_FORMAT_SECURITY:=0

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/comgt/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
endef

define Package/comgt
$(call Package/comgt/Default)
  TITLE:=Option/Vodafone 3G/GPRS control tool
  DEPENDS:=+chat
  URL:=http://manpages.ubuntu.com/manpages/trusty/man1/comgt.1.html
endef

define Package/comgt-directip
$(call Package/comgt/Default)
  TITLE:=Sierra Wireless Direct-IP support
  DEPENDS:=+comgt +kmod-usb-serial +kmod-usb-serial-sierrawireless +kmod-usb-net +kmod-usb-net-sierrawireless
endef

define Package/comgt-ncm
$(call Package/comgt/Default)
  TITLE+=NCM 3G/4G Support
  DEPENDS:=+comgt +wwan
endef

define Package/comgt/description
 comgt is a scripting language interpreter useful for establishing 
 communications on serial lines and through PCMCIA modems as well as GPRS 
 and 3G datacards.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="" \
		comgt
endef

define Package/comgt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/comgt $(1)/usr/bin/
	$(LN) comgt $(1)/usr/bin/gcom
	$(INSTALL_DIR) $(1)/etc/chatscripts
	$(INSTALL_DATA) ./files/3g.chat $(1)/etc/chatscripts/3g.chat
	$(INSTALL_DATA) ./files/evdo.chat $(1)/etc/chatscripts/evdo.chat
	$(INSTALL_DIR) $(1)/etc/gcom
	$(INSTALL_DATA) ./files/setpin.gcom $(1)/etc/gcom/setpin.gcom
	$(INSTALL_DATA) ./files/setmode.gcom $(1)/etc/gcom/setmode.gcom
	$(INSTALL_DATA) ./files/getcardinfo.gcom $(1)/etc/gcom/getcardinfo.gcom
	$(INSTALL_DATA) ./files/getstrength.gcom $(1)/etc/gcom/getstrength.gcom
	$(INSTALL_DATA) ./files/getcarrier.gcom $(1)/etc/gcom/getcarrier.gcom
	$(INSTALL_DATA) ./files/getcnum.gcom $(1)/etc/gcom/getcnum.gcom
	$(INSTALL_DATA) ./files/getimsi.gcom $(1)/etc/gcom/getimsi.gcom
	$(INSTALL_DIR) $(1)/etc/hotplug.d/tty
	$(INSTALL_DATA) ./files/3g.usb $(1)/etc/hotplug.d/tty/30-3g
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/3g.sh $(1)/lib/netifd/proto/3g.sh
endef

define Package/comgt-directip/install
	$(INSTALL_DIR) $(1)/etc/gcom
	$(INSTALL_DATA) ./files/directip.gcom $(1)/etc/gcom/directip.gcom
	$(INSTALL_DATA) ./files/directip-stop.gcom $(1)/etc/gcom/directip-stop.gcom
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/directip.sh $(1)/lib/netifd/proto/directip.sh
endef

define Package/comgt-ncm/install
	$(INSTALL_DIR) $(1)/etc/gcom
	$(INSTALL_DATA) ./files/ncm.json $(1)/etc/gcom/ncm.json
	$(INSTALL_DATA) ./files/runcommand.gcom $(1)/etc/gcom/runcommand.gcom
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/ncm.sh $(1)/lib/netifd/proto/ncm.sh
endef

$(eval $(call BuildPackage,comgt))
$(eval $(call BuildPackage,comgt-directip))
$(eval $(call BuildPackage,comgt-ncm))
