#
# Copyright (C) 2018-2020 The Regents of the University of California
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spoofer
PKG_VERSION:=1.4.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.caida.org/projects/spoofer/downloads
PKG_HASH:=fb814d0beaccc48a155e55bb1e7057520d0fe0584dbcc4c0a89f5cee55d0eaf1

PKG_MAINTAINER:=Ken Keys <spoofer-info@caida.org>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=protobuf/host

include $(INCLUDE_DIR)/package.mk

define Package/spoofer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Measure your ISP's resistance to spoofed IP packets
  URL:=https://spoofer.caida.org/
  DEPENDS:=+protobuf-lite +libpcap +libpthread +libopenssl
endef

define Package/spoofer/description
The spoofer client is part of a system to measure the Internet's
resistance to packets with a spoofed (forged) source IP address.

This package comes bundled with small certificate files that allow
secure communication with the spoofer server without depending on
the large ca-certificates package.  But if the server's private
certificate ever changes, it will be necessary to either install the
ca-certificates package or install an updated version of this package
that has a newer bundled certificate.

endef

CONFIGURE_ARGS += \
	--enable-prober \
	--disable-manager

CONFIGURE_VARS += \
	PROTOC=$(STAGING_DIR_HOSTPKG)/bin/protoc

define Package/spoofer/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin//spoofer-prober $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt-files/spoofer $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt-files/initscript $(1)/etc/init.d/spoofer
	$(INSTALL_DIR) $(1)/usr/share/spoofer
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/gd_bundle.crt $(1)/usr/share/spoofer
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/letsencrypt_bundle.pem.txt $(1)/usr/share/spoofer
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openwrt-files/spoofer-lib.sh $(1)/usr/share/spoofer
endef

$(eval $(call BuildPackage,spoofer))
