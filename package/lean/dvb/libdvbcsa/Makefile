#
# Copyright (C) 2014 nanpuyue <nanpuyue@gmail.com>
# Copyright (C) 2017 Tomasz Maciej Nowak <tomek_n@o2.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := libdvbcsa
PKG_RELEASE := 1

PKG_SOURCE_PROTO := git
PKG_SOURCE_URL := https://github.com/glenvt18/libdvbcsa.git
PKG_SOURCE_VERSION := 2a1e61e569a621c55c2426f235f42c2398b7f18f

PKG_VERSION := 1.1.0-$(shell printf '%.8s' $(PKG_SOURCE_VERSION))
#PKG_SOURCE := $(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL := http://download.videolan.org/pub/videolan/libdvbcsa/$(PKG_VERSION)/
#PKG_HASH := 4db78af5cdb2641dfb1136fe3531960a477c9e3e3b6ba19a2754d046af3f456d

PKG_LICENSE := GPL-2.0
PKG_LICENSE_FILE := COPYING

PKG_MAINTAINER := Tomasz Maciej Nowak <tomek_n@o2.pl>

PKG_INSTALL := 1

include $(INCLUDE_DIR)/package.mk

define Package/libdvbcsa
  SECTION := libs
  CATEGORY := Libraries
  TITLE := DVB Common Scrambling Algorithm library
  URL := https://www.videolan.org/developers/libdvbcsa.html
endef

define Package/libdvbcsa/description
 libdvbcsa is a free implementation of the DVB Common Scrambling
 Algorithm - DVB/CSA - with encryption and decryption capabilities
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); ./bootstrap)
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/
endef

define Package/libdvbcsa/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdvbcsa.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdvbcsa))
