#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=miniupnpd
PKG_VERSION:=2.0.20170421
PKG_RELEASE:=3

PKG_SOURCE_URL:=http://miniupnp.free.fr/files
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=9677aeccadf73b4bf8bb9d832c32b5da8266b4d58eed888f3fd43d7656405643

PKG_MAINTAINER:=Markus Stenberg <fingon@iki.fi>
PKG_LICENSE:=BSD-3-Clause

include $(INCLUDE_DIR)/package.mk

define Package/miniupnpd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+iptables +libip4tc +libuuid
  TITLE:=Lightweight UPnP IGD, NAT-PMP & PCP daemon
  SUBMENU:=Firewall
  URL:=http://miniupnp.free.fr/
endef

define Package/miniupnpd/config
config MINIUPNPD_IGDv2
	bool
	default n
	prompt "Enable IGDv2"
endef

define Package/miniupnpd/conffiles
/etc/config/upnpd
endef

define Package/miniupnpd/postinst
#!/bin/sh

if [ -z "$$IPKG_INSTROOT" ]; then
  ( . /etc/uci-defaults/99-miniupnpd )
  rm -f /etc/uci-defaults/99-miniupnpd
fi

exit 0
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	echo "OpenWrt" | tr \(\)\  _ >$(PKG_BUILD_DIR)/os.openwrt
endef

MAKE_FLAGS += \
	TARGET_OPENWRT=1 TEST=0 \
	LIBS="" \
	CC="$(TARGET_CC) -DIPTABLES_143 \
		-lip4tc -luuid" \
	CONFIG_OPTIONS="--portinuse --leasefile \
		$(if $(CONFIG_MINIUPNPD_IGDv2),--igd2)" \
	-f Makefile.linux \
	miniupnpd


define Package/miniupnpd/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/etc/uci-defaults $(1)/etc/hotplug.d/iface $(1)/usr/share/miniupnpd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/miniupnpd $(1)/usr/sbin/miniupnpd
	$(INSTALL_BIN) ./files/miniupnpd.init $(1)/etc/init.d/miniupnpd
	$(INSTALL_CONF) ./files/upnpd.config $(1)/etc/config/upnpd
	$(INSTALL_DATA) ./files/miniupnpd.hotplug $(1)/etc/hotplug.d/iface/50-miniupnpd
	$(INSTALL_DATA) ./files/miniupnpd.defaults $(1)/etc/uci-defaults/99-miniupnpd
	$(INSTALL_DATA) ./files/firewall.include $(1)/usr/share/miniupnpd/firewall.include
endef

$(eval $(call BuildPackage,miniupnpd))
