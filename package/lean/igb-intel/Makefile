#
# Drivers Copyright (C) 2022 Intel Corporation
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kmod-igb-intel
PKG_VERSION:=5.10.2
PKG_RELEASE:=1

PKG_SOURCE:=igb-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.sourceforge.net/project/e1000/igb%20stable/$(PKG_VERSION)/
PKG_MD5SUM:=6ef2c26c18acb898d693040b56b27316

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_UNPACK:=zcat $(DL_DIR)/$(PKG_SOURCE) | $(TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xf -

MAKE_PATH:=src

MAKE_VARS:= \
  KSRC="$(LINUX_DIR)" 

MAKE_OPTS:= \
  ARCH="$(LINUX_KARCH)" \
  CROSS_COMPILE="$(KERNEL_CROSS)" 

define KernelPackage/igb-intel
  SUBMENU:=Network Devices
  TITLE:=Intel igb drivers - oot version from Intel
  DEPENDS:=+kmod-ptp @PCI_SUPPORT 
  KCONFIG:=CONFIG_IGB \
    CONFIG_IGB_HWMON=n \
    CONFIG_IGB_DCA=n
  FILES:=$(PKG_BUILD_DIR)/src/igb.ko
  AUTOLOAD:=$(call AutoLoad,35,igb)
endef

define KernelPackage/igb-intel/description
 Kernel modules for Intel igb out of tree drivers
endef

$(eval $(call KernelPackage,igb-intel))
