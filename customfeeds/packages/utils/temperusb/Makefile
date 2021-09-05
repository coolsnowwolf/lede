#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=temperusb
PKG_VERSION:=2.3
PKG_RELEASE:=1
PKG_CONFIG_DEPENDS:=libusb

PKG_BUILD_DIR:=$(BUILD_DIR)/temperv14-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/Arduous/temperv14/tar.gz/v${PKG_VERSION}?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=3319026e0d5c42aebe9a75a23593f181c29d0ec9d544fac90b7ba886269ff246
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_MAINTAINER := Samuel Progin <samuel.progin@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/temperusb
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libusb-1.0
  TITLE:=USB Thermometer RDing TEMPer v1.4 reader
endef

define Package/temperusb/description
  RDing TEMPer v1.4 USB thermometer are cheap devices that can be sourced everywhere on
  the Internet. This package allows to operate them from user space.
endef

define Package/temperusb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/temperusb $(1)/usr/bin
endef

$(eval $(call BuildPackage,temperusb))

