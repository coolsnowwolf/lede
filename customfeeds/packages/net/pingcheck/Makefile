#
# Copyright (C) 2015 Bruno Randolf (br1@einfach.org)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pingcheck
PKG_VERSION:=2021-02-22
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/br101/pingcheck
PKG_SOURCE_VERSION:=c82e03ec536476b978aad2c23711450724b80b91
PKG_MIRROR_HASH:=6a81a1f6253bcb1dc64abb58e5f8113f1e0061a920c5a975ad34c06f5adc2719

PKG_MAINTAINER:=Bruno Randolf <br1@einfach.org>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pingcheck
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libubus +libuci
	MAINTAINER:=Bruno Randolf <br1@einfach.org>
	TITLE:=Check Internet and interface connectivity
	URL:=https://github.com/br101/pingcheck
endef

define Package/pingcheck/description
Checks by using "ping" (ICMP echo) or by opening connections to TCP port 80
whether a configured host (normally on the internet) can be reached via a
specific interface. Then makes this information available via ubus and triggers
"online" and "offline" scripts.
endef

define Package/pingcheck/conffiles
/etc/config/pingcheck
endef

define Package/pingcheck/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/pingcheck $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./pingcheck.init $(1)/etc/init.d/pingcheck
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/pingcheck.config $(1)/etc/config/pingcheck
	$(INSTALL_DIR) $(1)/etc/pingcheck/online.d/
	$(INSTALL_DIR) $(1)/etc/pingcheck/offline.d/
	$(INSTALL_DIR) $(1)/etc/pingcheck/panic.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/example-script.sh $(1)/etc/pingcheck/online.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/example-script.sh $(1)/etc/pingcheck/offline.d/
endef

$(eval $(call BuildPackage,pingcheck))
