#
# Copyright © 2020 David Woodhouse <dwmw2@infradead.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=cereal
PKG_VERSION:=1.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/USCiLab/cereal/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=329ea3e3130b026c03a4acc50e168e7daff4e6e661bc6a7dfec0d77b570851d5

PKG_MAINTAINER:=David Woodhouse <dwmw2@infradead.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
	-DCMAKE_CXX_FLAGS=-latomic \
	-DJUST_INSTALL_CEREAL=ON \
	-DSKIP_PORTABILITY_TEST=ON \
	-DSKIP_PERFORMANCE_COMPARISON=ON \
	-DWITH_WERROR=OFF

define Package/cereal
  BUILDONLY:=1
  SECTION:=devel
  CATEGORY:=Development
  SUBMENU:=Libraries
  TITLE:=Cereal is a library of C++ header files for serialization
  URL:=https://github.com/USCilab/cereal
endef


define Package/cereal/description
  Cereal is a library of C++ headers for serialization
endef

$(eval $(call BuildPackage,cereal))
