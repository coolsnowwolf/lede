include $(TOPDIR)/rules.mk

PKG_NAME:=libfstrm
PKG_VERSION:=0.6.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=fstrm-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://dl.farsightsecurity.com/dist/fstrm/
PKG_HASH:=a7049089eb0861ecaa21150a05613caa6dee4e8545b91191eff2269caa923910

PKG_BUILD_DIR:=$(BUILD_DIR)/fstrm-$(PKG_VERSION)

PKG_MAINTAINER:=Peter van Dijk <peter.van.dijk@powerdns.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libfstrm
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C implementation of the Frame Streams data transport protocol
  URL:=https://github.com/farsightsec/fstrm
endef

define Package/libfstrm/description
fstrm is an optimized C implementation of Frame Streams that includes a fast,
  lockless circular queue implementation and exposes library interfaces for
  setting up a dedicated Frame Streams I/O thread and asynchronously submitting
  data frames for transport from worker threads. It was originally written to
  facilitate the addition of high speed binary logging to DNS servers written
  in C using the dnstap log format.
endef

CONFIGURE_ARGS+= \
	--disable-programs

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/include/fstrm
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fstrm/*.h $(1)/usr/include/fstrm/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfstrm.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libfstrm.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libfstrm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfstrm.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libfstrm))
