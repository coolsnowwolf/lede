# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lib_ifxos
PKG_VERSION:=1.5.19
PKG_RELEASE:=4
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/xdarklight/$(PKG_NAME)/archive/v$(PKG_VERSION)
PKG_HASH:=ed7fe39311d7a4a13d23ed0ae2445c0d825b472b5a98da9b72bcaabcf5ed2d5f
PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0 BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-ifxos
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Libraries
  TITLE:=Lantiq OS abstraction library
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq
  FILES:=$(PKG_BUILD_DIR)/src/drv_ifxos.ko
  AUTOLOAD:=$(call AutoLoad,10,drv_ifxos)
endef

CONFIGURE_ARGS += \
	ARCH=$(LINUX_KARCH) \
	--enable-linux-26 \
	--enable-kernelbuild="$(LINUX_DIR)" \
	--enable-kernelincl="$(LINUX_DIR)/include" \
	--with-kernel-module

ifdef CONFIG_TARGET_lantiq
  define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include/ifxos}
	$(CP) $(PKG_BUILD_DIR)/src/include/* $(1)/usr/include/ifxos
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libifxos.a $(1)/usr/lib/libifxos.a
  endef
endif

$(eval $(call KernelPackage,ltq-ifxos))
