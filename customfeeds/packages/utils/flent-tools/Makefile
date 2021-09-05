#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flent-tools
PKG_VERSION:=1.3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tohojo/flent/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=f71793bb924b36d0e4c230182755c3f5dd26cd7f239b1fa6b717d942258cbf9a
PKG_BUILD_DIR:=$(BUILD_DIR)/flent-$(PKG_VERSION)

PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/flent-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Helper programs for the Flent test tool
  URL:=https://flent.org
  MAINTAINER:=Toke Høiland-Jørgensen <toke@toke.dk>
  DEPENDS+=+librt +coreutils-sleep +coreutils-date
endef

define Package/flent-tc_iterate/description
	This builds the C helper programs for gathering data for
	the Flent network testing tool. These tools are needed for
	gathering data when a Bash shell is not available (and also
	perform better than the shell equivalents).
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/misc \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -std=gnu99"
endef

define Package/flent-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/misc/tc_iterate $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/misc/wifistats_iterate $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,flent-tools))
