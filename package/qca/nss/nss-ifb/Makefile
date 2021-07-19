#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nss-ifb
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nss-ifb
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=NSS IFB Interface
  DEPENDS:=+kmod-qca-nss-drv @LINUX_5_4
  FILES:=$(PKG_BUILD_DIR)/nss-ifb.ko
  KCONFIG:=
endef

define KernelPackage/nss-ifb/description
 Kernel module to register a NSS aware IFB interface.
endef

EXTRA_KCONFIG:= \
	CONFIG_NET_CLS=y

EXTRA_CFLAGS:= \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,nss-ifb))

