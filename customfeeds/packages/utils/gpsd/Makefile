#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gpsd
PKG_VERSION:=3.21
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SAVANNAH/$(PKG_NAME)
PKG_HASH:=5512a7d3c2e86be83c5555652e5b4cc9049e8878a4320be7f039eb1a7203e5f0

PKG_MAINTAINER:=Pushpal Sidhu <psidhu.devel@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:berlios:gps_daemon

PKG_BUILD_DEPENDS:=scons/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/scons/scons.mk

define Package/gpsd/Default
  DEPENDS+= +libusb-1.0
  URL:=https://gpsd.gitlab.io/gpsd/
endef

define Package/gpsd/Default/description
  gpsd is a userland daemon acting as a translator between GPS and AIS receivers
  and their clients. gpsd listens on port 2947 for clients requesting
  position/time/velocity information.  The receivers are expected to generate
  position information in a well-known format -- as NMEA-0183 sentences, SiRF
  binary, Rockwell binary, Garmin binary format, or other vendor binary
  protocols.  gpsd takes this information from the GPS and translates it into
  something uniform and easier to understand for clients.
endef

define Package/gpsd
  $(call Package/gpsd/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=An interface daemon for GPS receivers
endef

define Package/gpsd/conffiles
/etc/config/gpsd
endef

define Package/gpsd/description
  $(call Package/gpsd/Default/description)
  This package contains the GPS daemon.
endef

define Package/gpsd-clients
  $(call Package/gpsd/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS+= +libncurses
  TITLE:=GPS tools and clients
endef

define Package/gpsd-clients/description
  $(call Package/gpsd/Default/description)
  This package contains auxiliary tools and example clients for monitoring and
  testing the GPS daemon.
endef

define Package/gpsd-utils
  $(call Package/gpsd/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GPS daemon utils
endef

define Package/gpsd-utils/description
  $(call Package/gpsd/Default/description)
  This package contains utilities for interacting with GPS daemon.
endef

define Package/libgps
  $(call Package/gpsd/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C service library for communicating with the GPS daemon
endef

define Package/libgps/description
  $(call Package/gpsd/Default/description)
  This package contains the libgps library.
endef

SCONS_VARS += \
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) -L$(STAGING_DIR)/usr/lib" \
	LINKFLAGS="$(TARGET_LDFLAGS)"

SCONS_OPTIONS += \
	dbus_export=no \
	tsip=no \
	fv18=no \
	tripmate=no \
	earthmate=no \
	itrax=no \
	navcom=no \
	ubx=no \
	evermore=no \
	ntrip=no \
	libgpsmm=no \
	libQgpsmm=no \
	bluez=no \
	nostrip=yes \
	python=no \
	implicit_link=no \
	chrpath=no \
	manbuild=no \
	sysroot="$(TOOLCHAIN_DIR)" \
	target="$(TARGET_CROSS:-=)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gps.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgps*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libgps.pc $(1)/usr/lib/pkgconfig
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libgps.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libgps.pc
endef

define Package/gpsd/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/gpsd.config $(1)/etc/config/gpsd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/gpsd.init $(1)/etc/init.d/gpsd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/gpsd $(1)/usr/sbin/
endef

define Package/gpsd-clients/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/cgps \
		$(PKG_INSTALL_DIR)/usr/bin/gps{ctl,decode,mon,pipe} \
		$(PKG_INSTALL_DIR)/usr/bin/gpxlogger \
		$(PKG_INSTALL_DIR)/usr/bin/lcdgps \
		$(1)/usr/bin/
endef

define Package/gpsd-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/gpsdctl $(1)/usr/sbin/
endef

define Package/libgps/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgps.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,gpsd))
$(eval $(call BuildPackage,gpsd-clients))
$(eval $(call BuildPackage,gpsd-utils))
$(eval $(call BuildPackage,libgps))
