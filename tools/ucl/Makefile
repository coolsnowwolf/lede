#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ucl
PKG_VERSION:=1.03

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/ucl/download/$(PKG_SOURCE)
PKG_HASH:=b865299ffd45d73412293369c9754b07637680e5c826915f097577cd27350348

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS +=-std=gnu89 -std=c90 -fPIC -Wno-error=implicit-function-declaration
HOST_CFLAGS +=-std=gnu89

define Host/Prepare
	$(Host/Prepare/Default)
	mkdir -p $(STAGING_DIR_HOST)/include/ucl
endef

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
	CC="$(HOSTCC)" \
	CFLAGS="$(HOST_CFLAGS)" \
	./configure --prefix=$(STAGING_DIR_HOST) \
	);
	$(call Host/Configure/Default)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/include/ucl
endef

$(eval $(call HostBuild))
