# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3+.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gkermit
PKG_VERSION:=1.00
PKG_RELEASE:=2

PKG_SOURCE:=gku100.tar.gz
PKG_SOURCE_URL:=https://ftp.nluug.nl/networking/kermit/archives
PKG_HASH:=3dbe63291277c4795255343b48b860777fb0a160163d7e1d30b1ee68585593eb

PKG_MAINTAINER:=Nathaniel Wesley Filardo <nwfilardo@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/gkermit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=gkermit
  URL:=http://www.columbia.edu/kermit/gkermit.html
endef

define Package/gkermit/description
  A minimal kermit file transfer program
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	tar -C $(PKG_BUILD_DIR) -xzvf "$(DL_DIR)/$(PKG_SOURCE)"
	$(Build/Patch)
endef

# nothing to do
define Build/Configure
endef
 
define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)
	$(TARGET_CC) \
	  $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
	  -include string.h -include unistd.h \
	  -DPOSIX -DERRNO_H \
	  -o $(PKG_INSTALL_DIR)/gkermit \
	  $(PKG_BUILD_DIR)/{gproto,gkermit,gunixio,gcmdline}.c
endef

define Package/gkermit/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/gkermit $(1)/bin
endef

$(eval $(call BuildPackage,gkermit))
