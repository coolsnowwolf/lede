#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libevent2
PKG_VERSION:=2.1.11
PKG_RELEASE:=2

PKG_SOURCE:=libevent-$(PKG_VERSION)-stable.tar.gz
PKG_SOURCE_URL:=https://github.com/libevent/libevent/releases/download/release-$(PKG_VERSION)-stable
PKG_HASH:=a65bac6202ea8c5609fd5c7e480e6d25de467ea1917c08290c521752f147283d
PKG_BUILD_DIR:=$(BUILD_DIR)/libevent-$(PKG_VERSION)-stable

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:libevent_project:libevent

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_libevent2-openssl \
	CONFIG_PACKAGE_libevent2-mbedtls

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libevent2/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Event notification
  URL:=http://libevent.org
  ABI_VERSION:=7
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
  TITLE+= library (version 2.1)
endef

define Package/libevent2/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent shared library historically
	containing both the core & extra libraries.
endef

define Package/libevent2-core
  $(call Package/libevent2/Default)
  TITLE+= core library (version 2.1)
endef

define Package/libevent2-core/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent core shared library for the event,
	buffer & utility functions.
endef

define Package/libevent2-extra
  $(call Package/libevent2/Default)
  TITLE+= extra library (version 2.1)
  DEPENDS+=+libevent2-core
endef

define Package/libevent2-extra/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent extra shared library for specific
	protocols including HTTP, DNS & RPC.
endef

define Package/libevent2-openssl
  $(call Package/libevent2/Default)
  TITLE+= OpenSSL library (version 2.1)
  DEPENDS+=+libopenssl +libevent2-core
endef

define Package/libevent2-openssl/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent OpenSSL shared library for encrypted
	bufferevents.
endef

define Package/libevent2-pthreads
  $(call Package/libevent2/Default)
  TITLE+= Pthreads library (version 2.1)
  DEPENDS+=+libpthread +libevent2-core
endef

define Package/libevent2-pthreads/description
	$(call Package/libevent2/Default/description)

	This package contains the libevent Pthreads shared library for
	threading & locking.
endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed -flto

CMAKE_OPTIONS += \
	-DEVENT__DISABLE_BENCHMARK:BOOL=ON \
	-DEVENT__DISABLE_DEBUG_MODE:BOOL=ON \
	-DEVENT__DISABLE_REGRESS:BOOL=ON \
	-DEVENT__DISABLE_SAMPLES:BOOL=ON \
	$(if $(CONFIG_PACKAGE_libevent2-openssl),-DEVENT__DISABLE_OPENSSL:BOOL=OFF,-DEVENT__DISABLE_OPENSSL:BOOL=ON) \
	$(if $(CONFIG_PACKAGE_libevent2-pthreads),-DEVENT__DISABLE_THREAD_SUPPORT:BOOL=OFF,-DEVENT__DISABLE_THREAD_SUPPORT:BOOL=ON) \
	-DEVENT__DISABLE_TESTS:BOOL=ON \
	-DBUILD_TESTING:BOOL=OFF

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libevent*.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libevent*.pc
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
