# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libdrm
PKG_VERSION:=2.4.104
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://dri.freedesktop.org/libdrm
PKG_HASH:=d66ad8b5c2441015ac1333e40137bb803c3bde3612ff040286fcc12158ea1bcb

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=BSD-3-Clause

PKG_CONFIG_DEPENDS:= \
	CONFIG_LIBDRM_INTEL \
	CONFIG_LIBDRM_NOUVEAU \
	CONFIG_LIBDRM_RADEON

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/libdrm
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+LIBDRM_INTEL:libpciaccess
  TITLE:=libdrm
  URL:=https://dri.freedesktop.org/
endef

define Package/libdrm/description
  LIBDRM is the cross-driver middleware which allows user-space applications
  (such as Mesa and 2D drivers) to communicate with the Kernel by the means
  of the DRI protocol.
endef

define Package/libdrm/config
	source "$(SOURCE)/Config.in"
endef

MESON_ARGS += \
	$(if $(CONFIG_LIBDRM_INTEL),-Dintel=true -Dlibkms=true,-Dintel=false -Dlibkms=false) \
	-Dradeon=$(if $(CONFIG_LIBDRM_RADEON),true,false) \
	-Damdgpu=false \
	-Dnouveau=$(if $(CONFIG_LIBDRM_NOUVEAU),true,false) \
	-Dvmwgfx=false \
	-Domap=false \
	-Dexynos=false \
	-Dfreedreno=false \
	-Dtegra=false \
	-Dvc4=false \
	-Detnaviv=false \
	-Dcairo-tests=false \
	-Dman-pages=false \
	-Dvalgrind=false \
	-Dfreedreno-kgsl=false \
	-Dinstall-test-programs=false \
	-Dudev=false

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endef

define Package/libdrm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdrm))
