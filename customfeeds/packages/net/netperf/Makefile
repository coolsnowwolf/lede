#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netperf
PKG_VERSION:=2.7.0
PKG_RELEASE:=2
PKG_LICENSE:=Custom

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.netperf.org/netperf/
PKG_HASH:=842af17655835c8be7203808c3393e6cb327a8067f3ed1f1053eb78b4e40375a

PKG_CPE_ID:=cpe:/a:netperf:netperf

include $(INCLUDE_DIR)/package.mk

define Package/netperf
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Network performance measurement tool
  URL:=http://www.netperf.org/
  MAINTAINER:=Toke Høiland-Jørgensen <toke@toke.dk>
endef

TARGET_CFLAGS+=-std=gnu89

CONFIGURE_ARGS += --enable-demo

define Package/netperf/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/netserver.init $(1)/etc/init.d/netserver
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/netperf $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/netserver $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/arr_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/packet_byte_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/sctp_stream_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/snapshot_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/tcp_range_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/tcp_rr_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/tcp_stream_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/udp_rr_script $(1)/lib/netperf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/doc/examples/udp_stream_script $(1)/lib/netperf
endef

$(eval $(call BuildPackage,netperf))
