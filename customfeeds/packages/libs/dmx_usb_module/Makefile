#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dmx_usb_module
PKG_VERSION:=19.12.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/lowlander/dmx_usb_module/tar.gz/V$(PKG_VERSION)?
PKG_HASH:=fabeb5500b55197338dda45d07374e5edc837bc18df3ac3114b442fe47cf1b68

PKG_MAINTAINER:=Martijn Zilverschoon <martijn@friedzombie.com>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-serial-dmx_usb_module
	SECTION:=kernel
	CATEGORY:=Kernel modules
	SUBMENU:=USB Support
	TITLE:=Support for FTDI RS485 based DMX modules
	URL:=http://www.erwinrol.com/open-dmx-usb-linux-driver/
	FILES:=$(PKG_BUILD_DIR)/dmx_usb.$(LINUX_KMOD_SUFFIX)
	AUTOLOAD:=$(call AutoProbe,dmx_usb)
	DEPENDS+=kmod-usb-serial
endef

define KernelPackage/usb-serial-dmx_usb_module/description
	Open DMX USB is an open USB to DMX dongle hardware design developed by Enttec.
	The Open in Open DMX USB refers to the fact that everybody is free to use the
	design and produce its own USB DMX Dongle without paying any licenses.
endef

DMX_MAKE_OPTS:= -C $(PKG_BUILD_DIR) \
	PATH="$(TARGET_PATH)" \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	TARGET="$(HAL_TARGET)" \
	TOOLPREFIX="$(KERNEL_CROSS)" \
	TOOLPATH="$(KERNEL_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	LDOPTS=" "

define Build/Compile
  $(MAKE) $(DMX_MAKE_OPTS) M=$(PKG_BUILD_DIR)
endef

$(eval $(call KernelPackage,usb-serial-dmx_usb_module))
