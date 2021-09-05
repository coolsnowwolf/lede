#
# Copyright (C) 2019 Banglang Huang <banglang.huang@foxmail.com>
# Copyright (C) 2019 Rosy Song <rosysong@rosinson.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kea
PKG_VERSION:=1.8.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://ftp.isc.org/isc/kea/$(PKG_VERSION)
PKG_HASH:=486ca7abedb9d6fdf8e4344ad8688d1171f2ef0f5506d118988aadeae80a1d39

PKG_MAINTAINER:=BangLang Huang<banglang.huang@foxmail.com>, Rosy Song<rosysong@rosinson.com>
PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=COPYING

HOST_BUILD_DEPENDS:=boost/host log4cplus/host
PKG_BUILD_DEPENDS:=kea/host
HOST_BUILD_PARALLEL:=1
PKG_BUILD_PARALLEL:=1

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/kea/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=ISC Kea
  URL:=https://www.isc.org/kea
endef


###### *************************************************************************
define Package/kea
	$(call Package/kea/Default)
  TITLE+=ISC Kea
endef
# shown in LuCI package description
define Package/kea/description
  Kea is an open source DHCPv4/DHCPv6 server being developed by Internet
  Systems Consortium. Kea is a high-performance, extensible DHCP server
  engine that is designed to be easily modified and extended with hooks
  libraries. It provides DHCPv4 and DHCPv6 servers, a dynamic DNS update
  module, a portable DHCP library, libdhcp++, control agent that provides
  management REST interface, and a DHCP benchmarking tool, perfdhcp.
endef

###### *************************************************************************
define Package/kea-libs
	$(call Package/kea/Default)
	TITLE+=Libraries
	DEPENDS:=+libopenssl +log4cplus \
	+boost +boost-system
endef
define Package/kea-libs/description
		Kea required Libraries.
endef

###### *************************************************************************
define Package/kea-ctrl
	$(call Package/kea/Default)
	TITLE+=Control
	DEPENDS:=+procps-ng +procps-ng-ps +kea-dhcp4 \
	+IPV6:kea-dhcp6 +kea-dhcp-ddns
endef
define Package/kea-ctrl/description
	Tool to start, stop, reconfigure, and report status for the Kea servers.
endef

###### *************************************************************************
define Package/kea-dhcp4
	$(call Package/kea/Default)
	TITLE+=DHCP Server v4
	DEPENDS:=+kea-libs
endef
define Package/kea-dhcp4/description
    The DHCPv4 server process. This process responds to DHCPv4 queries from clients.
endef

###### *************************************************************************
define Package/kea-dhcp6
	$(call Package/kea/Default)
	TITLE+=DHCP Server v6
	DEPENDS:=@IPV6 +kea-libs
endef
define Package/kea-dhcp6/description
    The DHCPv6 server process. This process responds to DHCPv6 queries from clients.
endef

###### *************************************************************************
define Package/kea-dhcp-ddns
	$(call Package/kea/Default)
	TITLE+=DHCP Dynamic DNS
	DEPENDS:=+kea-libs
endef
define Package/kea-dhcp-ddns/description
		The DHCP Dynamic DNS process. This process acts as an intermediary between
the DHCP servers and DNS servers. It receives name update requests from the
DHCP servers and sends DNS Update messages to the DNS servers.
endef

###### *************************************************************************
define Package/kea-admin
	$(call Package/kea/Default)
	TITLE+=Admin
	DEPENDS:=+kea-libs
endef
define Package/kea-admin/description
   	A useful tool for database backend maintenance (creating a new database,
checking versions, upgrading etc.).
endef

##### *************************************************************************
define Package/kea-lfc
	$(call Package/kea/Default)
	TITLE+=lfc
	DEPENDS:=+kea-libs
endef
define Package/kea-lfc/description
    This process removes redundant information from the files used to provide
persistent storage for the memfile data base backend. While it can be run
standalone, it is normally run as and when required by the Kea DHCP servers.
endef

##### *************************************************************************
define Package/kea-perfdhcp
	$(call Package/kea/Default)
	TITLE+=perfdhcp (Benchmarking)
	DEPENDS:=+kea-libs
endef
define Package/kea-perfdhcp/description
     A DHCP benchmarking tool which simulates multiple clients to test both
DHCPv4 and DHCPv6 server performance.
endef

###### *************************************************************************
define Package/kea-shell
	$(call Package/kea/Default)
	TITLE+=shell
	DEPENDS:=+kea-libs
endef
define Package/kea-shell/description
     This simple text client uses the REST interface to connect to the Kea
Control Agent.
endef

###### *************************************************************************
HOST_CONFIGURE_ARGS += \
	--with-boost-include="$(STAGING_DIR_HOSTPKG)" \
	--with-log4cplus="$(STAGING_DIR_HOSTPKG)" \
	--with-openssl="$(STAGING_DIR_HOST)" \
	--enable-generate-messages

HOST_LDFLAGS += \
	 -Wl,--gc-sections,--as-needed

CONFIGURE_ARGS += \
	--with-boost-include="$(STAGING_DIR)/usr" \
	--with-log4cplus="$(STAGING_DIR)/usr" \
	--with-openssl="$(STAGING_DIR)/usr" \
	--without-pic \
	$(if $(CONFIG_PACKAGE_kea-perfdhcp),--enable-perfdhcp,) \
	$(if $(CONFIG_PACKAGE_kea-shell),--enable-shell,)

CONFIGURE_VARS += \
	cross_compiling="yes"

TARGET_CXXFLAGS += \
	$(FPIC) \
	-fdata-sections \
	-ffunction-sections \
	-std=c++17

TARGET_LDFLAGS += \
	-Wl,--gc-sections,--as-needed

# Only compile the kea-msg-compiler which we need for
# package compilation
define Host/Compile
	+$(HOST_MAKE_VARS) \
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/src/lib/exceptions $(HOST_MAKE_FLAGS)
	+$(HOST_MAKE_VARS) \
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/src/lib/util $(HOST_MAKE_FLAGS)
	+$(HOST_MAKE_VARS) \
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/src/lib/log $(HOST_MAKE_FLAGS)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/lib/log/compiler/kea-msg-compiler \
		$(STAGING_DIR_HOSTPKG)/bin/
endef

define Build/Compile
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src/lib/log/compiler
	$(INSTALL_BIN) $(STAGING_DIR_HOSTPKG)/bin/kea-msg-compiler \
		$(PKG_BUILD_DIR)/src/lib/log/compiler/
	$(call Build/Compile/Default)
endef

define Package/kea-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

define Package/kea-dhcp4/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/kea
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-dhcp4 $(1)/usr/sbin/kea-dhcp4
	$(CP) $(PKG_INSTALL_DIR)/etc/kea/kea-dhcp4.conf $(1)/etc/kea/
endef

define Package/kea-dhcp6/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/kea
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-dhcp6 $(1)/usr/sbin/kea-dhcp6
	$(CP) $(PKG_INSTALL_DIR)/etc/kea/kea-dhcp6.conf $(1)/etc/kea/
endef

define Package/kea-dhcp-ddns/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/kea
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-dhcp-ddns $(1)/usr/sbin/kea-dhcp-ddns
	$(CP) $(PKG_INSTALL_DIR)/etc/kea/kea-dhcp-ddns.conf $(1)/etc/kea/
endef

define Package/kea-admin/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-admin $(1)/usr/sbin/kea-admin
endef

define Package/kea-ctrl/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/kea
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/keactrl $(1)/usr/sbin/keactrl
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-ctrl-agent $(1)/usr/sbin/kea-ctrl-agent
	$(CP) $(PKG_INSTALL_DIR)/etc/kea/keactrl.conf $(1)/etc/kea/
	$(CP) $(PKG_INSTALL_DIR)/etc/kea/kea-ctrl-agent.conf $(1)/etc/kea/
endef

define Package/kea-lfc/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-lfc $(1)/usr/sbin/kea-lfc
endef

define Package/kea-perfdhcp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/perfdhcp $(1)/usr/sbin/perfdhcp
endef

define Package/kea-shell/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kea-shell $(1)/usr/sbin/kea-shell
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,kea-libs))
$(eval $(call BuildPackage,kea-ctrl))
$(eval $(call BuildPackage,kea-dhcp4))
$(eval $(call BuildPackage,kea-dhcp6))
$(eval $(call BuildPackage,kea-dhcp-ddns))
$(eval $(call BuildPackage,kea-admin))
$(eval $(call BuildPackage,kea-lfc))
$(eval $(call BuildPackage,kea-perfdhcp))
$(eval $(call BuildPackage,kea-shell))
