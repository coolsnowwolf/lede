#
# Copyright (C) 2020 Lucian Cristian
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcap-ng
PKG_VERSION:=0.8.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://people.redhat.com/sgrubb/libcap-ng
PKG_HASH:=f06b17aaca029e245c9a26c698c6cc8a1cf42b58483d93e94ee02b478bdc1055

PKG_MAINTAINER:=Lucian CRISTIAN <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later LGPL-2.1-or-later
PKG_LICENSE_FILES:=License

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/libcap-ng/Default
  TITLE:=POSIX capabilities library
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://people.redhat.com/sgrubb/libcap-ng/index.html
endef

define Package/libcap-ng/description/Default
  POSIX capabilities library
endef

define Package/libcap-ng
  $(call Package/libcap-ng/Default)
  TITLE += library
endef

define Package/libcap-ng-bin
  $(call Package/libcap-ng/Default)
  TITLE += binaries
  DEPENDS += libcap-ng
endef

define Package/libcap-ng-bin/description
  $(call Package/libcap-ng/description/Default)
  .
  This package contains the libcap-ng utilities.
endef

TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed $(FPIC)
TARGET_CFLAGS += -flto

CONFIGURE_VARS += \
    ac_cv_prog_swig_found=no

CONFIGURE_ARGS += \
    --prefix=/usr \
    --without-python \
    --without-python3

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcap-ng.{so*,a} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libcap-ng.pc $(1)/usr/lib/pkgconfig/
	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/cap-ng.m4\
	 $(1)/usr/share/aclocal
endef

define Package/libcap-ng/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcap-ng.so* $(1)/usr/lib/
endef

define Package/libcap-ng-bin/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,libcap-ng))
$(eval $(call BuildPackage,libcap-ng-bin))
