#
# Copyright (C) Chen Yijun
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=xr-usb-serial
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=USB Support
  DEPENDS:=@GPIO_SUPPORT +kmod-usb-serial +kmod-usb2
  TITLE:=Driver for XR21V141x (used in HUAWEI UPS)
  AUTOLOAD:=$(call AutoLoad,30,xr_usb_serial_common,1)
  FILES:=$(PKG_BUILD_DIR)/xr_usb_serial_common.ko
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
