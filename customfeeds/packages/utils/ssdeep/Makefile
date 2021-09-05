#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ssdeep
PKG_VERSION:=2.14.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ssdeep-project/ssdeep/releases/download/release-$(PKG_VERSION)/
PKG_HASH:=ff2eabc78106f009b4fb2def2d76fb0ca9e12acf624cbbfad9b3eb390d931313

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>

include $(INCLUDE_DIR)/package.mk

define Package/ssdeep
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A program to compute and match fuzzy hashes
  URL:=https://ssdeep-project.github.io/ssdeep
  DEPENDS:=+libstdcpp
endef

define Package/ssdeep/description
 ssdeep is a program for computing context triggered piecewise hashes (CTPH). Also called fuzzy hashes, CTPH can match inputs that have homologies.
endef

define Package/ssdeep/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ssdeep $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ssdeep))
