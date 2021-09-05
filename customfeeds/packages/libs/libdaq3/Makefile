#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdaq3
PKG_VERSION:=3.0.3
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_LICENSE:=GPL-2.0-only

PKG_SOURCE:=libdaq-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/snort3/libdaq/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=920344f5c98ac68b401d1bc92ebaed78b0d15779b0480c213db279d0a60acc32
PKG_BUILD_DIR:=$(BUILD_DIR)/libdaq-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdaq3
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=DAQ library
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=+libdnet +libpcap +libstdcpp
endef

define Package/libdaq3/description
 Data Acquisition library for packet I/O.
endef

CONFIGURE_ARGS+= \
	--disable-static \
	--disable-nfq-module \
	--with-dnet-includes="$(STAGING_DIR)/usr/include" \
	--with-dnet-libraries="$(STAGING_DIR)/usr/lib" \
	--with-libpcap-includes="$(STAGING_DIR)/usr/include" \
	--with-libpcap-libraries="$(STAGING_DIR)/usr/lib" \

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/daq3
	$(CP) $(PKG_INSTALL_DIR)/usr/include/. $(STAGING_DIR)/usr/include/daq3/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib/daq3
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(STAGING_DIR)/usr/lib/daq3/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib/daq3/daq
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/daq/* $(STAGING_DIR)/usr/lib/daq3/daq/
endef

define Package/libdaq3/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/daq
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/daq/*.so* $(1)/usr/lib/daq/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libdaq3))
