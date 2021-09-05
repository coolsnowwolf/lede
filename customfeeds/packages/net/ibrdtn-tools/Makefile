#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ibrdtn-tools
PKG_VERSION:=1.0.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
PKG_HASH:=eab066cf15f9c322d769c6c9c58adfb474cba7d446fd12e8de5ff6344376795b
PKG_MAINTAINER:=Johannes Morgenroth <jm@m-network.de>
PKG_LICENSE:=Apache-2.0

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ibrdtn-tools
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+ibrdtn +libarchive
  TITLE:=DTN Tools
endef

define Package/ibrdtn-tools/description
 The IBR-DTN Tools include functionality for sending and receiving files (dtnsend/dtnrecv)
 and a tools to ping a DTN node (dtnping).
endef

CONFIGURE_ARGS += \
	--without-libdaemon

TARGET_CXXFLAGS += -std=c++11

define Package/ibrdtn-tools/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtnping $(1)/usr/bin/dtnping
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtnsend $(1)/usr/bin/dtnsend
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtnrecv $(1)/usr/bin/dtnrecv
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtninbox $(1)/usr/bin/dtninbox
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtnoutbox $(1)/usr/bin/dtnoutbox
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtntrigger $(1)/usr/bin/dtntrigger
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtntracepath $(1)/usr/bin/dtntracepath
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtnstream $(1)/usr/bin/dtnstream
endef

$(eval $(call BuildPackage,ibrdtn-tools))
