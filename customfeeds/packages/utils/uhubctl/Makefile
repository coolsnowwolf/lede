include $(TOPDIR)/rules.mk

PKG_NAME:=uhubctl
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mvp/uhubctl/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=391f24fd1f89cacce801df38ecc289b34c3627bc08ee69eec515af7e1a283d97

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING LICENSE

PKG_MAINTAINER:=Steven Honson <steven@honson.id.au>

include $(INCLUDE_DIR)/package.mk

define Package/uhubctl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=USB hub per-port power control
  URL:=https://github.com/mvp/uhubctl
  DEPENDS:=+libusb-1.0
endef

define Package/uhubctl/description
  uhubctl is a utility that allows you to control USB power on USB hubs that
  implement per-port or ganged power switching.
endef

define Package/uhubctl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uhubctl $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,uhubctl))
