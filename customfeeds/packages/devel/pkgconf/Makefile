#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pkgconf
PKG_VERSION:=1.7.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://distfiles.dereferenced.org/pkgconf
PKG_HASH:=d73f32c248a4591139a6b17777c80d4deab6b414ec2b3d21d0a24be348c476ab

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/libpkgconf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libpkgconf
  URL:=http://pkgconf.org/
endef

define Package/pkgconf
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=pkgconf
  URL:=http://pkgconf.org/
  DEPENDS:=+libpkgconf
endef

define Package/libpkgconf/description
  libpkgconf is a library which provides access to most of pkgconf’s
  functionality, to allow other tooling such as compilers and IDEs to
  discover and use frameworks configured by pkgconf. It features a stable
  library ABI and API designed for building bindings and other tools.
endef

define Package/pkgconf/description
  pkgconf is a program which helps to configure compiler and linker flags
  for development frameworks. It is similar to pkg-config from
  freedesktop.org, providing additional functionality while also
  maintaining compatibility.
endef

MESON_ARGS += \
	-Dtests=false

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libpkgconf
	$(CP) $(PKG_INSTALL_DIR)/usr/include/pkgconf/libpkgconf/*.h $(1)/usr/include/libpkgconf
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpkgconf.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share/aclocal/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/pkg.m4 \
	  $(1)/usr/share/aclocal/
endef

define Package/libpkgconf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpkgconf.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share/aclocal/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/pkg.m4 \
	  $(1)/usr/share/aclocal/
endef

define Package/pkgconf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pkgconf $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libpkgconf))
$(eval $(call BuildPackage,pkgconf))
