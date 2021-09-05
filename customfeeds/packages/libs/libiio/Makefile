#
# Copyright (C) 2017-2018 Michael Heimpold <mhei@heimpold.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libiio
PKG_VERSION:=0.21
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/analogdevicesinc/libiio/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=03d13165cbeb83b036743cbd9a10e336c728da162714f39d13250a3d94305cac

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING.txt

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

CMAKE_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_IPV6 \
	CONFIG_LIBIIO_LOCAL_BACKEND \
	CONFIG_LIBIIO_NETWORK_BACKEND \
	CONFIG_LIBIIO_USB_BACKEND \
	CONFIG_LIBIIO_XML_BACKEND

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DWITH_DOC=OFF
CMAKE_OPTIONS += -DENABLE_IPV6=$(if $(CONFIG_IPV6),ON,OFF)
CMAKE_OPTIONS += -DENABLE_AIO=OFF
CMAKE_OPTIONS += -DWITH_LOCAL_BACKEND=$(if $(CONFIG_LIBIIO_LOCAL_BACKEND),ON,OFF)
CMAKE_OPTIONS += -DWITH_LOCAL_CONFIG=OFF
CMAKE_OPTIONS += -DWITH_NETWORK_BACKEND=$(if $(CONFIG_LIBIIO_NETWORK_BACKEND),ON,OFF)
# serial backend requires libserial which is not packaged yet
CMAKE_OPTIONS += -DWITH_SERIAL_BACKEND=OFF
CMAKE_OPTIONS += -DWITH_USB_BACKEND=$(if $(CONFIG_LIBIIO_USB_BACKEND),ON,OFF)
CMAKE_OPTIONS += -DWITH_XML_BACKEND=$(if $(CONFIG_LIBIIO_XML_BACKEND),ON,OFF)

define Package/libiio
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for interfacing with Linux IIO devices
  URL:=https://github.com/analogdevicesinc/libiio
  DEPENDS:=\
           +zlib \
           +LIBIIO_USB_BACKEND:libusb-1.0 \
           +LIBIIO_NETWORK_BACKEND:libavahi-client \
           +LIBIIO_XML_BACKEND:libxml2 \
           $(ICONV_DEPENDS)
  MENU:=1
endef

define Package/libiio/config
config LIBIIO_LOCAL_BACKEND
	bool "Enable local backend"
	depends on PACKAGE_libiio
	default y

config LIBIIO_NETWORK_BACKEND
	bool "Enable network backend"
	depends on PACKAGE_libiio
	select LIBIIO_XML_BACKEND
	default y

config LIBIIO_USB_BACKEND
	bool "Enable USB backend"
	depends on PACKAGE_libiio
	select LIBIIO_XML_BACKEND
	default n

config LIBIIO_XML_BACKEND
	bool
	depends on PACKAGE_libiio
	default y
endef

define Package/libiio/description
  libiio is used to interface to the Linux Industrial Input/Output (IIO) Subsystem.
  The Linux IIO subsystem is intended to provide support for devices that in some
  sense are analog to digital or digital to analog converters (ADCs, DACs). This
  includes, but is not limited to ADCs, Accelerometers, Gyros, IMUs, Capacitance
  to Digital Converters (CDCs), Pressure Sensors, Color, Light and Proximity Sensors,
  Temperature Sensors, Magnetometers, DACs, DDS (Direct Digital Synthesis),
  PLLs (Phase Locked Loops), Variable/Programmable Gain Amplifiers (VGA, PGA),
  and RF transceivers. You can use libiio natively on an embedded Linux
  target (local mode), or use libiio to communicate remotely to that same target
  from a host Linux, Windows or MAC over USB or Ethernet or Serial.
endef

define Package/iiod
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Linux IIO daemon
  URL:=https://github.com/analogdevicesinc/libiio
  DEPENDS:=+libiio
endef

define Package/iiod/description
  Daemon to access IIO devices via network.
endef

define Package/iio-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Linux IIO tools
  URL:=https://github.com/analogdevicesinc/libiio
  DEPENDS:=+libiio
endef

define Package/iio-utils/description
  Command line tools for IIO devices.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/iio.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libiio.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libiio.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libiio/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libiio.so.* $(1)/usr/lib/
endef

define Package/iiod/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/iiod.init $(1)/etc/init.d/iiod
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/iiod $(1)/usr/sbin/
endef

define Package/iio-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libiio))
$(eval $(call BuildPackage,iiod))
$(eval $(call BuildPackage,iio-utils))
