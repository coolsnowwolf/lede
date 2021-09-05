include $(TOPDIR)/rules.mk

PKG_NAME:=netwhere
PKG_VERSION:=0.9
PKG_RELEASE:=2

PKG_SOURCE:=netwhere-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/benhsmith/netwhere/archive/$(PKG_VERSION)/
PKG_HASH:=94a672bdcd9d4455b85429dddd81ffc778e0b26fe87af19ad75c27858ec9dbe2

PKG_MAINTAINER:=Ben Smith <le.ben.smith@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=boost

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/netwhere
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Netwhere
  URL:=https://github.com/benhsmith/netwhere
  DEPENDS:=+libtins +libmicrohttpd
endef

define Package/netwhere/description
  Network monitor
endef

define Package/netwhere/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netwhere $(1)/usr/bin/netwhere
	$(INSTALL_DIR) $(1)/www/netwhere
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/html/index.html $(1)/www/netwhere/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/html/netwhere.js $(1)/www/netwhere/
endef

$(eval $(call BuildPackage,netwhere))
