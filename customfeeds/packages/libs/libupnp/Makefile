include $(TOPDIR)/rules.mk

PKG_NAME:=libupnp
PKG_VERSION:=1.14.7
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/pupnp
PKG_HASH:=7b66ac4a86bc0e218e2771ac274b2945bc4154bf9054e57b14afb67c26ac7c24

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libupnp_project:libupnp

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_libupnp-sample \
	CONFIG_IPV6

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libupnp/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://pupnp.sourceforge.net/
endef

define Package/libupnp
  $(call Package/libupnp/Default)
  DEPENDS:=+libpthread
  TITLE:=UPnP SDK library
  MENU:=1
endef

define Package/libupnp/description
The portable SDK for UPnP Devices (libupnp) provides developers with an API and
open source code for building control points, devices, and bridges that are
compliant with Version 1.0 of the  Universal Plug and Play Device Architecture
Specification.
endef

define Package/libupnp-sample
  $(call Package/libupnp/Default)
  DEPENDS:=libupnp
  TITLE:=UPnP sample applications
endef

define Package/libupnp-sample/description
TVcontrolpoint & tvdevice sample applications run inside /etc/upnp-tvdevice/
endef

CMAKE_OPTIONS += \
	-DBUILD_TESTING=OFF \
	-Dclient=ON \
	-Ddevice=ON \
	-Dwebserver=OFF \
	-Dssdp=ON \
	-Doptssdp=OFF \
	-Dsoap=ON \
	-Dgena=ON \
	-Dtools=ON \
	-Dipv6=O$(if $(CONFIG_IPV6),N,FF) \
	-Dunspecified_server=OFF \
	-Dopen_ssl=OFF \
	-Dblocking_tcp_connections=ON \
	-Dscriptsupport=OFF \
	-Dpostwrite=OFF \
	-Dreuseaddr=ON \
	-Dsamples=O$(if $(CONFIG_PACKAGE_libupnp-sample),N,FF) \
	-DDOWNLOAD_AND_BUILD_DEPS=OFF

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/upnp $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{ixml,upnp}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/cmake
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/* $(1)/usr/lib/cmake
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libupnp.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libupnp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{ixml,upnp}.so.* $(1)/usr/lib/
endef

define Package/libupnp-sample/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tv* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share/upnp
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/upnp/tv* $(1)/usr/share/upnp
endef

$(eval $(call BuildPackage,libupnp))
$(eval $(call BuildPackage,libupnp-sample))
