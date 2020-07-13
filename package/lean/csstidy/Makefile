include $(TOPDIR)/rules.mk

PKG_NAME:=csstidy
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/jow-/csstidy-cpp.git
PKG_SOURCE_DATE:=2018-12-20
PKG_SOURCE_VERSION:=1d5620149ae35c0d49fb2014d4e63a23ecfb6f69
PKG_MIRROR_HASH:=3210d475f6ae966d4dfcd3e1f7fcbf0ad9507a37878d50de015ffe795c1d160e

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
