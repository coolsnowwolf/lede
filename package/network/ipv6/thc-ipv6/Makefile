#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=thc-ipv6
PKG_VERSION:=3.8
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GITHUB/vanhauser-thc/THC-Archive/master/Tools
PKG_HASH:=b60be61a8b0a944a66e3b719704b4c03c1bc2c22f32d5d21e99e434c82a9d769

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

include $(INCLUDE_DIR)/package.mk

THC_APPLETS := \
	address6 alive6 covert_send6 covert_send6d denial6 detect-new-ip6     \
	detect_sniffer6 dnsdict6 dnsrevenum6 dos-new-ip6                      \
	dump_router6 exploit6 fake_advertise6 fake_dhcps6 fake_dns6d          \
	fake_dnsupdate6 fake_mipv6 fake_mld26 fake_mld6 fake_mldrouter6       \
	fake_router26 fake_router6 fake_solicitate6 flood_advertise6          \
	flood_dhcpc6 flood_mld26 flood_mld6 flood_mldrouter6 flood_router26   \
	flood_router6 flood_solicitate6 fragmentation6 fuzz_ip6 fuzz_dhcpc6   \
	fuzz_dhcps6 implementation6 implementation6d inverse_lookup6          \
	kill_router6 ndpexhaust6 node_query6 parasite6 passive_discovery6     \
	randicmp6 redir6 rsmurf6 sendpees6 sendpeesmp6 smurf6 thcping6        \
	toobig6 trace6 toobigsniff6 flood_unreach6 connect6

THC_DEPENDS_dnsdict6 := +libpthread
THC_DEPENDS_thcping6 := +librt

define BuildTool
  define Package/thc-ipv6-$(subst _,-,$(1))
    TITLE:=THC-IPv6 $(1) utility
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+libpcap $(THC_DEPENDS_$(1))
    URL:=https://github.com/vanhauser-thc/thc-ipv6
    SUBMENU:=THC-IPv6 attack and analyzing toolkit
  endef

  define Package/thc-ipv6-$(subst _,-,$(1))/description
    This package contains the $(1) utility of the THC-IPv6 toolkit.
  endef

  define Package/thc-ipv6-$(subst _,-,$(1))/install
	$(INSTALL_DIR) $$(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1) $$(1)/usr/sbin/$(1)
  endef

  $$(eval $$(call BuildPackage,thc-ipv6-$(subst _,-,$(1))))
endef

$(foreach a,$(THC_APPLETS),$(eval $(call BuildTool,$(a))))
