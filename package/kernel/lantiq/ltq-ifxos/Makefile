# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ifxos
PKG_VERSION:=1.7.1
PKG_RELEASE:=$(AUTORELEASE)

UGW_VERSION=8.5.2.10
UGW_BASENAME=$(PKG_NAME)-ugw_$(UGW_VERSION)

PKG_SOURCE:=$(UGW_BASENAME).tar.bz2
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(UGW_BASENAME)
PKG_SOURCE_URL:=https://gitlab.com/prpl-foundation/intel/$(PKG_NAME)/-/archive/ugw_$(UGW_VERSION)/
PKG_HASH:=055a1f5eab0abfaac34ac7b1613b93ec341fe9ae8462cb11c36c2b0989ce0ca7
PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0 BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_EXTMOD_SUBDIRS:=src

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
	$(CP) $(PKG_BUILD_DIR)/src/.libs/libifxos.a $(1)/usr/lib/libifxos.a
  endef
endif

$(eval $(call KernelPackage,ltq-ifxos))
