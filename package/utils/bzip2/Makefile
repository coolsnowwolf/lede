#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bzip2
PKG_VERSION:=1.0.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://sourceware.org/pub/bzip2
PKG_HASH:=ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269

PKG_MAINTAINER:=Steven Barth <cyrus@openwrt.org>
PKG_LICENSE:=bzip2-1.0.8
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:bzip:bzip2

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/bzip2/Default
  SUBMENU:=Compression
  URL:=https://sourceware.org/bzip2/
endef

define Package/libbz2
$(call Package/bzip2/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=
  TITLE:=bzip2 library.
  ABI_VERSION:=1.0
endef

define Package/libbz2/description
	bzip2 is a freely available, patent free, high-quality
	data compressor. This packages provides libbz2 library.
endef

define Package/bzip2
$(call Package/bzip2/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libbz2
  TITLE:=bzip2 is a compression utility.
endef

define Package/bzip2/description
	bzip2 is a freely available, patent free, high-quality
	data compressor. This package provides the binary.
endef

TARGET_CFLAGS += \
	$(FPIC)

CONFIGURE_ARGS += --prefix=/usr

MAKE_FLAGS += \
	-f Makefile-libbz2_so \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/bzlib.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libbz2.so.$(PKG_VERSION) $(1)/usr/lib/
	$(LN) libbz2.so.$(PKG_VERSION) $(1)/usr/lib/libbz2.so.1.0
	$(LN) libbz2.so.$(PKG_VERSION) $(1)/usr/lib/libbz2.so
endef

define Package/libbz2/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libbz2.so.$(PKG_VERSION) $(1)/usr/lib/
	$(LN) libbz2.so.$(PKG_VERSION) $(1)/usr/lib/libbz2.so.1.0
endef

define Package/bzip2/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bzip2-shared $(1)/usr/bin/bzip2
	$(INSTALL_DIR) $(1)/bin/
	$(LN) ../usr/bin/bzip2 $(1)/bin/bzip2
endef

HOST_CFLAGS += \
	$(FPIC)

HOST_MAKE_FLAGS+= \
	CFLAGS="$(HOST_CFLAGS)" \
	LDFLAGS="$(HOST_LDFLAGS)" \
	all

HOST_CONFIGURE_ARGS+= \
	--prefix=$(STAGING_DIR_HOSTPKG)

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin/
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX=$(STAGING_DIR_HOSTPKG)/ install
endef

$(eval $(call HostBuild))

$(eval $(call BuildPackage,libbz2))
$(eval $(call BuildPackage,bzip2))
