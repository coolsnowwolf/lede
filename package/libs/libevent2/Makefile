#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libevent2
PKG_VERSION:=2.1.8
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/libevent/libevent.git
PKG_SOURCE_VERSION:=release-2.1.8-stable
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=libevent-$(PKG_VERSION)-stable.tar.gz
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=BSD-3-Clause

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libevent2/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Event notification
  URL:=http://www.monkey.org/~provos/libevent/
  DEPENDS+=+libopenssl
endef

define Package/libevent2/Default/description
	The libevent API provides a mechanism to execute a callback function
	when a specific event occurs on a file descriptor or after a timeout
	has been reached. Furthermore, libevent also support callbacks due
	to signals or regular timeouts.

	libevent is meant to replace the event loop found in event driven
	network servers. An application just needs to call event_dispatch()
	and then add or remove events dynamically without having to change
	the event loop.
endef

define Package/libevent2
  $(call Package/libevent2/Default)
  TITLE+= library (version 2.0)
endef

define Package/libevent2/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent shared library historically
	containing both the core & extra libraries.
endef

define Package/libevent2-core
  $(call Package/libevent2/Default)
  TITLE+= core library (version 2.0)
endef

define Package/libevent2-core/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent core shared library for the event,
	buffer & utility functions.
endef

define Package/libevent2-extra
  $(call Package/libevent2/Default)
  TITLE+= extra library (version 2.0)
endef

define Package/libevent2-extra/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent extra shared library for specific
	protocols including HTTP, DNS & RPC.
endef

define Package/libevent2-openssl
  $(call Package/libevent2/Default)
  TITLE+= OpenSSL library (version 2.0)
endef

define Package/libevent2-openssl/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent OpenSSL shared library for encrypted
	bufferevents.
endef

define Package/libevent2-pthreads
  $(call Package/libevent2/Default)
  TITLE+= Pthreads library (version 2.0)
  DEPENDS+=+libpthread
endef

define Package/libevent2-pthreads/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent Pthreads shared library for
	threading & locking.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-debug-mode

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent*.{la,a,so} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent*-2.1.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libevent*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libevent2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent-2.1.so.* $(1)/usr/lib/
endef

define Package/libevent2-core/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent_core-2.1.so.* $(1)/usr/lib/
endef

define Package/libevent2-extra/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent_extra-2.1.so.* $(1)/usr/lib/
endef

define Package/libevent2-openssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent_openssl-2.1.so.* $(1)/usr/lib/
endef

define Package/libevent2-pthreads/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libevent_pthreads-2.1.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libevent2))
$(eval $(call BuildPackage,libevent2-core))
$(eval $(call BuildPackage,libevent2-extra))
$(eval $(call BuildPackage,libevent2-openssl))
$(eval $(call BuildPackage,libevent2-pthreads))
