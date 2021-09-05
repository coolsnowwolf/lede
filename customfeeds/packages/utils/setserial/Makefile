include $(TOPDIR)/rules.mk

PKG_NAME:=setserial
PKG_VERSION:=2.17
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/setserial
PKG_HASH:=7e4487d320ac31558563424189435d396ddf77953bb23111a17a3d1487b5794a

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/setserial
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://setserial.sourceforge.net/
  TITLE:=Serial port attribute utility
endef

define Package/setserial/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setserial $(1)/bin/
endef

define Package/setserial/description
 Setserial is a program which allows you to look at and change various
 attributes of a serial device, including its port, its IRQ, and other
 serial port options.
endef

$(eval $(call BuildPackage,setserial))
