include $(TOPDIR)/rules.mk

PKG_NAME:=which
PKG_VERSION:=2.21
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/which
PKG_HASH:=f4a245b94124b377d8b49646bf421f9155d36aa7614b6ebf83705d3ffc76eaad

PKG_MAINTAINER:=Huangbin Zhan <zhanhb88@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/which
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=which utility - full version
  URL:=https://carlowood.github.io/which/
  ALTERNATIVES:=300:/usr/bin/which:/usr/libexec/which-gnu
endef

define Package/which/description
 The which command shows the full pathname of a specified program, if the specified program is in your PATH.
endef

define Package/which/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/which $(1)/usr/libexec/which-gnu
endef

$(eval $(call BuildPackage,which))
