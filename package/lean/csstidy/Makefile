include $(TOPDIR)/rules.mk

PKG_NAME:=csstidy
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/jow-/csstidy-cpp.git
PKG_SOURCE_DATE:=2021-06-13
PKG_SOURCE_VERSION:=707feaec556c40c999514a598b1a1ea5b50826c6
PKG_MIRROR_HASH:=a6db35de692d1b94561e725634ce410b75f5f7e26dc6f3fc92931813c0721ded

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/csstidy
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CSSTidy parser and optimiser
  DEPENDS:=+libstdcpp
endef

define Package/csstidy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/csstidy/csstidy $(1)/usr/bin/
endef


define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/csstidy/csstidy $(1)/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,csstidy))
