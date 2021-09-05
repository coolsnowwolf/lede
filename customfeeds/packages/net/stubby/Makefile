# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stubby
PKG_VERSION:=0.4.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/getdnsapi/$(PKG_NAME)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=bc5f604da1b70287a6c3d89eac2e13ce8bca52840e7b72ab098a3deeb9935082

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/stubby/Default
  TITLE:=stubby
  URL:=https://dnsprivacy.org/wiki/display/DP/DNS+Privacy+Daemon+-+Stubby
endef

define Package/stubby
  $(call Package/stubby/Default)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE+= - (daemon that uses getdns)
  USERID:=stubby=410:stubby=410
  DEPENDS:= +libyaml +getdns +ca-certs
endef

define Package/stubby/description
  This package contains the Stubby daemon (which utilizes the getdns library).
  See https://github.com/openwrt/packages/blob/master/net/stubby/files/README.md for more details.
endef

define Package/stubby/conffiles
/etc/stubby/stubby.yml
/etc/config/stubby
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	rm $(PKG_BUILD_DIR)/cmake/modules/FindLibidn2.cmake $(PKG_BUILD_DIR)/cmake/modules/FindLibunbound.cmake
endef

define Package/stubby/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stubby $(1)/usr/sbin/stubby
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/stubby.init $(1)/etc/init.d/stubby
	$(INSTALL_DIR) $(1)/etc/stubby
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/stubby/stubby.yml $(1)/etc/stubby/stubby.yml.default
	$(INSTALL_DATA) ./files/stubby.yml $(1)/etc/stubby/stubby.yml
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/stubby.conf $(1)/etc/config/stubby
endef

$(eval $(call BuildPackage,stubby))
