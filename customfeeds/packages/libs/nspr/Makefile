#
# Copyright (C) 2019 Lucian Cristian
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=nspr
PKG_VERSION:=4.30
PKG_RELEASE:=1
PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENCE:=MPL-2.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
    https://download.cdn.mozilla.net/pub/$(PKG_NAME)/releases/v$(PKG_VERSION)/src/ \
    https://archive.mozilla.org/pub/$(PKG_NAME)/releases/v$(PKG_VERSION)/src/
PKG_HASH:=8d4cd8f8409484dc4c3d31e180354bfc506573eccf86cd691106a1ef7edc913b

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

CONFIGURE_PATH = ./nspr
MAKE_PATH = ./nspr
PKG_AUTOMAKE_PATHS = $(PKG_BUILD_DIR)/nspr
LBITS = $(shell $(TARGET_CC) -dM -E - </dev/null | grep -q "__LP64__" && echo 64 || echo 32)

ifeq ($(LBITS),64)
    conf=--enable-64bit
endif

export MUSL=$(if $(CONFIG_LIBC_USE_GLIBC),0,1)
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed $(FPIC)
TARGET_CFLAGS += -flto

CONFIGURE_ARGS += \
    --build=$(GNU_HOST_NAME) \
    --host=$(GNU_HOST_NAME) \
    --target=$(REAL_GNU_TARGET_NAME) \
    --disable-debug \
    --with-pthreads \
    $(if $(CONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
    $(conf)

define Build/Compile
	CROSS_COMPILE=1 CFLAGS="-DXP_UNIX $(HOST_CFLAGS)" LDFLAGS="" CC="$(HOSTCC)" \
	    $(MAKE) -C $(PKG_BUILD_DIR)/nspr/config
	$(call Build/Compile/Default)
endef

define Package/nspr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Netscape Portable Runtime (NSPR)
  URL:=https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSPR
  DEPENDS:=+libpthread
endef

define Package/nspr/description
  Netscape Portable Runtime (NSPR) provides a platform-neutral API for system
  level and libc-like functions. The API is used in the Mozilla clients, many
  of Red Hat's and Oracle's server applications, and other software offerings.
endef

define Build/InstallDev
	$(INSTALL_DIR) \
	    $(1)/usr/include \
	    $(1)/usr/lib/ \
	    $(1)/usr/lib/pkgconfig/ \
	    $(1)/usr/share/aclocal/

	$(CP) $(PKG_INSTALL_DIR)/usr/include/nspr $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/* $(1)/usr/share/aclocal/
endef

define Package/nspr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,nspr))
