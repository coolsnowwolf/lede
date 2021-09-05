#
# Copyright (C) 2014 Openwrt.org
#
# This is free software, licensed under the GNU General Public License v2.

include $(TOPDIR)/rules.mk

PKG_NAME:=bcp38
PKG_VERSION:=5
PKG_RELEASE:=6
PKG_LICENCE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/bcp38
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=BCP38 compliance
  URL:=https://github.com/dtaht/ceropackages-3.10
  MAINTAINER:=Toke Høiland-Jørgensen <toke@toke.dk>
  DEPENDS:=+ipset
  PKGARCH:=all
endef

define Package/bcp38/description
 bcp38 implements IETF BCP38 for home routers.
 See https://tools.ietf.org/html/bcp38.

 This package provides BCP38 for IPv4 only - IPv6 uses source 
 specific default routes, so no firewall configuration is needed.
endef

define Package/bcp38/conffiles
/etc/config/bcp38
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/bcp38/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/bcp38.config $(1)/etc/config/bcp38
	$(INSTALL_DIR) $(1)/usr/lib/bcp38
	$(INSTALL_BIN) ./files/run.sh $(1)/usr/lib/bcp38/run.sh
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/bcp38.defaults $(1)/etc/uci-defaults/bcp38
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bcp38.init $(1)/etc/init.d/bcp38
endef

define Package/bcp38/postinst
#!/bin/sh
[ -x /etc/uci-defaults/bcp38 ] && /etc/uci-defaults/bcp38 || exit 0
endef

define Package/bcp38/postrm
#!/bin/sh
uci delete firewall.bcp38
uci commit
endef

$(eval $(call BuildPackage,bcp38))
