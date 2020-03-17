#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=odhcpd
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/odhcpd.git
PKG_SOURCE_DATE:=2020-01-14
PKG_SOURCE_VERSION:=6db312a698e920ff61505ef1f42469880829774d
PKG_MIRROR_HASH:=4c9f5effd379fed3119f0a2d2661c5929ac234d9b2e3d7a6a1de72409b04a0f1

PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

PKG_INSTALL:=1
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_odhcpd_$(BUILD_VARIANT)_ext_cer_id
PKG_ASLR_PIE_REGULAR:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/odhcpd/default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=OpenWrt DHCPv6(-PD)/RA Server & Relay
  DEPENDS:=+libubox +libuci +libubus +libnl-tiny
endef

define Package/odhcpd/default/description
 odhcpd is a daemon for serving and relaying IP management protocols to
 configure clients and downstream routers. It tries to follow the RFC 6204
 requirements for IPv6 home routers.
endef

define Package/odhcpd/default/config
menu "Configuration"
	depends on PACKAGE_$(1)

config PACKAGE_odhcpd_$(2)_ext_cer_id
	int
	default 0
	prompt "CER-ID Extension ID (0 = disabled)"
endmenu
endef

define Package/odhcpd
  $(call Package/odhcpd/default)
  TITLE += and DHCPv4 server
  VARIANT:=full
endef

Package/odhcpd/config=$(call Package/odhcpd/default/config,odhcpd,full)

define Package/odhcpd/description
 $(call Package/odhcpd/default/description)

 This is a variant providing server services for DHCPv4, RA, stateless and
 stateful DHCPv6,  prefix delegation and can be used to relay RA, DHCPv6 and
 NDP between routed (non-bridged) interfaces in case no delegated prefixes
 are available.
endef

define Package/odhcpd-ipv6only
  $(call Package/odhcpd/default)
  VARIANT:=ipv6only
  DEPENDS+= @IPV6
endef

Package/odhcpd-ipv6only/config=$(call Package/odhcpd/default/config,odhcpd-ipv6only,ipv6only)

define Package/odhcpd-ipv6only/description
 $(call Package/odhcpd/default/description)

 This is a variant providing server services for RA, stateless and stateful
 DHCPv6,  prefix delegation and can be used to relay RA, DHCPv6 and NDP between
 routed (non-bridged) interfaces in case no delegated prefixes are available.
endef

CMAKE_OPTIONS += -DUBUS=1

ifeq ($(BUILD_VARIANT),full)
  CMAKE_OPTIONS += -DDHCPV4_SUPPORT=1
endif

ifneq ($(CONFIG_PACKAGE_odhcpd_$(BUILD_VARIANT)_ext_cer_id),0)
  CMAKE_OPTIONS += -DEXT_CER_ID=$(CONFIG_PACKAGE_odhcpd_$(BUILD_VARIANT)_ext_cer_id)
endif

define Package/odhcpd/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/odhcpd $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/odhcpd-update $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/odhcpd.init $(1)/etc/init.d/odhcpd
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/odhcpd.defaults $(1)/etc/uci-defaults
endef

Package/odhcpd-ipv6only/install = $(Package/odhcpd/install)

$(eval $(call BuildPackage,odhcpd))
$(eval $(call BuildPackage,odhcpd-ipv6only))
