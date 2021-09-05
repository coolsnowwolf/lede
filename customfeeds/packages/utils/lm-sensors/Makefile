#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lm-sensors
PKG_VERSION:=3.6.0
PKG_RELEASE:=1

PKG_VERSION_SUBST=$(subst .,-,$(PKG_VERSION))
PKG_SOURCE_URL:=https://codeload.github.com/lm-sensors/lm-sensors/tar.gz/V$(PKG_VERSION_SUBST)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=0591f9fa0339f0d15e75326d0365871c2d4e2ed8aa1ff759b3a55d3734b7d197
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION_SUBST)

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0-or-later LGPL-2.1-or-later
PKG_CPE_ID:=cpe:/a:lm_sensors:lm_sensors

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lm-sensors/Default
  DEPENDS:=+sysfsutils
  URL:=https://hwmon.wiki.kernel.org/lm_sensors
endef

define Package/lm-sensors
  $(call Package/lm-sensors/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=lm-sensors
  DEPENDS+=+libsensors
endef

define Package/lm-sensors-detect
  $(call Package/lm-sensors/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=lm-sensors-detect
  DEPENDS+=+lm-sensors \
	+PACKAGE_lm-sensors-detect:perl \
	+PACKAGE_lm-sensors-detect:perlbase-essential \
	+PACKAGE_lm-sensors-detect:perlbase-fcntl \
	+PACKAGE_lm-sensors-detect:perlbase-file \
	+PACKAGE_lm-sensors-detect:perlbase-xsloader
endef

define Package/libsensors
  $(call Package/lm-sensors/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libsensors
  ABI_VERSION:=5
endef

define Package/lm-sensors/description
 utility to read hardware sensor data
endef

define Package/lm-sensors-detect/description
 script to autodetect sensor hardware
endef

define Package/libsensors/description
 lm-sensors libraries
endef

define Package/lm-sensors/conffiles
/etc/sensors.conf
/etc/sensors3.conf
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		KERNELVERSION="$(LINUX_VERSION)" \
		LINUX="$(LINUX_DIR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		PREFIX="/usr" \
		MACHINE="$(ARCH)" \
		EXLDFLAGS="" \
		user
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/sensors
	$(CP) \
		$(PKG_BUILD_DIR)/lib/sensors.h \
		$(1)/usr/include/sensors
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_BUILD_DIR)/lib/libsensors.{a,so*} \
		$(1)/usr/lib
endef

define Package/lm-sensors/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/prog/sensors/sensors $(1)/usr/sbin
ifeq ($(ARCH),i386)
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/sensors.conf $(1)/etc/sensors.conf
endif
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/lm-sensors.init $(1)/etc/init.d/lm-sensors
endef

define Package/lm-sensors-detect/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/prog/detect/sensors-detect $(1)/usr/sbin
endef

define Package/libsensors/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libsensors.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libsensors))
$(eval $(call BuildPackage,lm-sensors))
$(eval $(call BuildPackage,lm-sensors-detect))
