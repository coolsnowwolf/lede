#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dnsmasq
PKG_VERSION:=2.80
PKG_RELEASE:=13

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://thekelleys.org.uk/dnsmasq
PKG_HASH:=cdaba2785e92665cf090646cba6f94812760b9d7d8c8d0cfb07ac819377a63bb

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:thekelleys:dnsmasq

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE_REGULAR:=1
PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dhcp \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dhcpv6 \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dnssec \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_auth \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_ipset \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_conntrack \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_noid \
	CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_broken_rtc

include $(INCLUDE_DIR)/package.mk

define Package/dnsmasq/Default
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=DNS and DHCP server
  URL:=http://www.thekelleys.org.uk/dnsmasq/
  DEPENDS:=+libubus
  USERID:=dnsmasq=453:dnsmasq=453
endef

define Package/dnsmasq
$(call Package/dnsmasq/Default)
  VARIANT:=nodhcpv6
endef

define Package/dnsmasq-dhcpv6
$(call Package/dnsmasq/Default)
  TITLE += (with DHCPv6 support)
  DEPENDS+=@IPV6
  VARIANT:=dhcpv6
  PROVIDES:=dnsmasq
endef

define Package/dnsmasq-full
$(call Package/dnsmasq/Default)
  TITLE += (with DNSSEC, DHCPv6, Auth DNS, IPset, Conntrack, NO_ID enabled by default)
  DEPENDS+=+PACKAGE_dnsmasq_full_dnssec:libnettle \
	+PACKAGE_dnsmasq_full_ipset:kmod-ipt-ipset \
	+PACKAGE_dnsmasq_full_conntrack:libnetfilter-conntrack
  VARIANT:=full
  PROVIDES:=dnsmasq
endef

define Package/dnsmasq/description
  It is intended to provide coupled DNS and DHCP service to a LAN.
endef

define Package/dnsmasq-dhcpv6/description
$(call Package/dnsmasq/description)

This is a variant with DHCPv6 support
endef

define Package/dnsmasq-full/description
$(call Package/dnsmasq/description)

This is a fully configurable variant with DHCPv4, DHCPv6, DNSSEC, Authoritative DNS
and IPset, Conntrack support & NO_ID enabled by default.
endef

define Package/dnsmasq/conffiles
/etc/config/dhcp
/etc/dnsmasq.conf
endef

define Package/dnsmasq-full/config
	if PACKAGE_dnsmasq-full
	config PACKAGE_dnsmasq_full_dhcp
		bool "Build with DHCP support."
		default y
	config PACKAGE_dnsmasq_full_dhcpv6
		bool "Build with DHCPv6 support."
		depends on IPV6 && PACKAGE_dnsmasq_full_dhcp
		default n
	config PACKAGE_dnsmasq_full_dnssec
		bool "Build with DNSSEC support."
		default n
	config PACKAGE_dnsmasq_full_auth
		bool "Build with the facility to act as an authoritative DNS server."
		default n
	config PACKAGE_dnsmasq_full_ipset
		bool "Build with IPset support."
		default y
	config PACKAGE_dnsmasq_full_conntrack
		bool "Build with Conntrack support."
		default n
	config PACKAGE_dnsmasq_full_noid
		bool "Build with NO_ID. (hide *.bind pseudo domain)"
		default n
	config PACKAGE_dnsmasq_full_broken_rtc
		bool "Build with HAVE_BROKEN_RTC."
		default n
	endif
endef

Package/dnsmasq-dhcpv6/conffiles = $(Package/dnsmasq/conffiles)
Package/dnsmasq-full/conffiles = $(Package/dnsmasq/conffiles)

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

COPTS = -DHAVE_UBUS \
	$(if $(CONFIG_IPV6),,-DNO_IPV6)

ifeq ($(BUILD_VARIANT),nodhcpv6)
	COPTS += -DNO_DHCP6
endif

ifeq ($(BUILD_VARIANT),full)
	COPTS += $(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dhcp),,-DNO_DHCP) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dhcpv6),,-DNO_DHCP6) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_dnssec),-DHAVE_DNSSEC) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_auth),,-DNO_AUTH) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_ipset),,-DNO_IPSET) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_conntrack),-DHAVE_CONNTRACK,) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_noid),-DNO_ID,) \
		$(if $(CONFIG_PACKAGE_dnsmasq_$(BUILD_VARIANT)_broken_rtc),-DHAVE_BROKEN_RTC)
	COPTS += $(if $(CONFIG_LIBNETTLE_MINI),-DNO_GMP,)
else
	COPTS += -DNO_AUTH -DNO_IPSET -DNO_ID
endif

MAKE_FLAGS := \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	COPTS="$(COPTS)" \
	PREFIX="/usr"

define Package/dnsmasq/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/dnsmasq $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/dhcp.conf $(1)/etc/config/dhcp
	$(INSTALL_CONF) ./files/dnsmasq.conf $(1)/etc/dnsmasq.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dnsmasq.init $(1)/etc/init.d/dnsmasq
	$(INSTALL_DIR) $(1)/etc/hotplug.d/dhcp
	$(INSTALL_DIR) $(1)/etc/hotplug.d/neigh
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ntp
	$(INSTALL_DIR) $(1)/etc/hotplug.d/tftp
	$(INSTALL_CONF) ./files/dnsmasqsec.hotplug $(1)/etc/hotplug.d/ntp/25-dnsmasqsec
	$(INSTALL_DIR) $(1)/usr/share/dnsmasq
	$(INSTALL_CONF) ./files/dhcpbogushostname.conf $(1)/usr/share/dnsmasq/
	$(INSTALL_CONF) ./files/rfc6761.conf $(1)/usr/share/dnsmasq/
	$(INSTALL_DIR) $(1)/usr/lib/dnsmasq
	$(INSTALL_BIN) ./files/dhcp-script.sh $(1)/usr/lib/dnsmasq/dhcp-script.sh
	$(INSTALL_DIR) $(1)/usr/share/acl.d
	$(INSTALL_DATA) ./files/dnsmasq_acl.json $(1)/usr/share/acl.d/
endef

Package/dnsmasq-dhcpv6/install = $(Package/dnsmasq/install)

define Package/dnsmasq-full/install
$(call Package/dnsmasq/install,$(1))
ifneq ($(CONFIG_PACKAGE_dnsmasq_full_dnssec),)
	$(INSTALL_DIR) $(1)/usr/share/dnsmasq
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/trust-anchors.conf $(1)/usr/share/dnsmasq
endif
endef

$(eval $(call BuildPackage,dnsmasq))
$(eval $(call BuildPackage,dnsmasq-dhcpv6))
$(eval $(call BuildPackage,dnsmasq-full))
