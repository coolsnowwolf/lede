#
# Copyright (C) 2015-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmraa
PKG_VERSION:=2.2.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/eclipse/mraa/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=076669bee8423ffef3065735b293a329020be86630fea457174dbfcc67a0554a
PKG_BUILD_DIR:=$(BUILD_DIR)/mraa-$(PKG_VERSION)

PKG_MAINTAINER:=John Crispin <blogic@openwrt.org>, Hirokazu MORIKAWA <morikw2@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=swig/host
CMAKE_INSTALL:=1
PKG_USE_MIPS16:=0
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include ../../lang/python/python3-package.mk

CMAKE_OPTIONS += \
	-DENABLEEXAMPLES=0 \
	-DBUILDSWIGNODE=OFF \
	-DFIRMATA=ON

define Package/libmraa/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=IoT
  TITLE:=Eclipse MRAA lowlevel IO library
  URL:=https://projects.eclipse.org/projects/iot.mraa
endef

define Package/libmraa/Default/description
  Libmraa is a C/C++ library with bindings to Java, Python and JavaScript to interface
with the IO on Galileo, Edison & other platforms, with a structured and sane API where
port names/numbering matches the board that you are on. Use of libmraa does not tie you
to specific hardware with board detection done at runtime you can create portable code
that will work across the supported platforms.
endef

define Package/libmraa
  $(call Package/libmraa/Default)
  TITLE:=Eclipse MRAA lowlevel IO C/C++ library
  DEPENDS:=+libstdcpp +libjson-c @!arc @!armeb @!powerpc
endef

define Package/libmraa/description
$(call Package/libmraa/Default/description)

This package contains the C/C++ libraries.
endef

define Package/libmraa-python3
  $(call Package/libmraa/Default)
  TITLE:=Eclipse MRAA lowlevel IO Python3 library
  DEPENDS:=+libmraa +python3-light
endef

define Package/libmraa-python3/description
$(call Package/libmraa/Default/description)

This package contains the Python3 libraries.
endef

define Package/libmraa/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmraa.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mraa-* $(1)/usr/bin/
endef

define Package/libmraa-python3/install
	$(INSTALL_DIR) $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION)/site-packages/* \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/
endef

$(eval $(call BuildPackage,libmraa))
$(eval $(call BuildPackage,libmraa-python3))
