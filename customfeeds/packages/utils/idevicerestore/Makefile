#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=idevicerestore
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.libimobiledevice.org/downloads
PKG_HASH:=32712e86315397fd2e8999e77a2d2f790c67f6b4aa50d4d1c64cb2c4609836f7

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/idevicerestore
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=libimobiledevice
  TITLE:=Restore/upgrade firmware of iOS devices
  URL:=https://github.com/libimobiledevice/idevicerestore
  DEPENDS:=+libirecovery +libzip +libcurl +usbmuxd
endef

define Package/idevicerestore/description
  The idevicerestore tool allows to restore firmware files to iOS devices.
  It is a full reimplementation of all granular steps which are performed
  during restore of a firmware to a device.
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)

define Package/idevicerestore/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/idevicerestore $(1)/usr/bin/
endef

$(eval $(call BuildPackage,idevicerestore))
