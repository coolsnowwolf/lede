#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcpreplay
PKG_VERSION:=4.3.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/appneta/tcpreplay/releases/download/v$(PKG_VERSION)
PKG_HASH:=42c055106e55852c29d94bb6e1b9e001a0723349f2985eb893a47d384c85002b

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=docs/LICENSE
PKG_CPE_ID:=cpe:/a:appneta:tcpreplay

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

TCPREPLAY_MODULES:= \
	tcpbridge tcpcapinfo tcpliveplay tcpprep \
	tcpreplay tcpreplay-edit tcprewrite

define Package/tcpreplay/default
  SUBMENU:=Tcpreplay
  SECTION:=net
  CATEGORY:=Network
  URL:=http://tcpreplay.appneta.com/
  MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
  DEPENDS:=+librt +libpcap +libdnet
endef

define Package/tcpbridge
$(call Package/tcpreplay/default)
  TITLE:=Bridge network traffic across two interfaces
endef

define Package/tcpcapinfo
$(call Package/tcpreplay/default)
  TITLE:=Pcap file dissector for debugging broken pcap files
endef

define Package/tcpliveplay
$(call Package/tcpreplay/default)
  TITLE:=Replays network traffic stored in a pcap file on live networks using new TCP connections
endef

define Package/tcpprep
$(call Package/tcpreplay/default)
  TITLE:=Create a tcpreplay cache file from a pcap file
endef

define Package/tcpreplay
$(call Package/tcpreplay/default)
  TITLE:=Replay network traffic stored in pcap files
endef

define Package/tcpreplay-edit
$(call Package/tcpreplay/default)
  TITLE:=Replay network traffic stored in pcap files
endef

define Package/tcprewrite
$(call Package/tcpreplay/default)
  TITLE:=Rewrite the packets in a pcap file
endef

define Package/tcpreplay-all
$(call Package/tcpreplay/default)
  TITLE:=Pcap editing and replaying utilities
  DEPENDS:=$(foreach m,$(TCPREPLAY_MODULES),+$(m))
endef

define Package/tcpbridge/description
  tcpbridge is a tool for selectively briding network traffic across two
  interfaces and optionally modifying the packets in between
endef

define Package/tcpcapinfo/description
  tcpcapinfo is a tool for decoding the structure of a pcap(3) file with a
  focus on finding broken pcap files and determining how two related pcap
  files might differ.
endef

define Package/tcpliveplay/description
  This program, 'tcpliveplay' replays a captured set of packets using new TCP
  connections with the captured TCP payloads against a remote host in order
  to do comprehensive vulnerability testings.
endef

define Package/tcpprep/description
  tcpprep is a ``pcap(3)'' file pre-processor which creates a cache file
  which provides "rules" for ``tcprewrite(1)'' and ``tcpreplay(1)'' on how to
  process and send packets.
endef

define Package/tcpreplay/description
  tcpreplay is a tool for replaying network traffic from files saved with
  tcpdump or other tools which write pcap(3) files.
endef

define Package/tcpreplay-edit/description
  tcpreplay-edit includes all the functionality of both tcpreplay
  and tcprewrite.
endef

define Package/tcprewrite/description
  Rewrite/edit the packets in a pcap file
endef

define Package/tcpreplay-all/description
  Tcpreplay is a suite of free Open Source utilities for
  editing and replaying previously captured network traffic.
  Originally designed to replay malicious traffic patterns to
  Intrusion Detection/Prevention Systems, it has seen many evolutions
  including capabilities to replay to web servers.

  Version 4.0.0 introduces features and performance enhancements
  to support switches, routers, and IP Flow/NetFlow appliances. 
endef

CONFIGURE_VARS += \
	ac_cv_lib_nl_nl_cache_alloc=no \
	ac_cv_lib_nl_genl_3_genl_connect=no \
	ac_cv_lib_nl_3_nl_cache_alloc=no \
	ac_cv_lib_dbus_1_dbus_malloc=no

CONFIGURE_ARGS += \
	--enable-force-pf \
	--enable-dynamic-link \
	--with-libdnet="$(STAGING_DIR)/usr" \
	--with-libpcap="$(STAGING_DIR)/usr"

define tcpreplayTemplate
  define Package/$(1)/install
    $(INSTALL_DIR) $$(1)/usr/bin ; \
    $(CP) $$(PKG_INSTALL_DIR)/usr/bin/$(1) $$(1)/usr/bin
  endef
endef

define Package/tcpreplay-all/install
	:
endef

$(foreach m, $(TCPREPLAY_MODULES), \
	$(eval $(call tcpreplayTemplate,$(m))) \
	$(eval $(call BuildPackage,$(m))) \
)

$(eval $(call BuildPackage,tcpreplay-all))

