# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libowfat
PKG_VERSION:=0.32
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.fefe.de/libowfat
PKG_HASH:=f4b9b3d9922dc25bc93adedf9e9ff8ddbebaf623f14c8e7a5f2301bfef7998c1
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS += libowfat/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

# set to 1 to enable debugging
DEBUG=

define Package/libowfat
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=reimplemented libdjb under GPL
  URL:=https://www.fefe.de/libowfat/
  BUILDONLY:=1
endef

define Build/Configure
endef

TARGET_CFLAGS += $(FPIC)
LOWFAT_MAKEOPTS = $(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(PKG_BUILD_DIR) -I$(STAGING_DIR)/usr/include" \
	DEBUG="$(DEBUG)" \
	VERSION="$(PKG_VERSION)" \
	OS="Linux"

LOWFAT_HOST_MAKEOPTS = $(HOST_CONFIGURE_OPTS) \
	CFLAGS="$(HOST_CFLAGS) -I$(HOST_BUILD_DIR) -I$(STAGING_DIR_HOSTPKG)/include" \
	DEBUG="$(DEBUG)" \
	VERSION="$(PKG_VERSION)" \
	OS="Linux"

# work around a nasty gcc bug
ifneq ($(CONFIG_GCC_VERSION_4_2_4),)
  LOWFAT_MAKEOPTS += WOPTS=""
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(LOWFAT_MAKEOPTS)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) $(LOWFAT_HOST_MAKEOPTS) ent
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin
	$(CP) $(HOST_BUILD_DIR)/ent $(STAGING_DIR_HOSTPKG)/bin/libowfat-ent
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include/libowfat
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/libowfat
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.a $(1)/usr/lib
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libowfat))
