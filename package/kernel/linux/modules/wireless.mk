#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

WIRELESS_MENU:=Wireless Drivers

define KernelPackage/net-prism54
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Intersil Prism54 support
  DEPENDS:=@PCI_SUPPORT +@DRIVER_WEXT_SUPPORT +prism54-firmware
  KCONFIG:=CONFIG_PRISM54
  FILES:= \
	$(LINUX_DIR)/drivers/net/wireless/prism54/prism54.ko@lt4.5 \
	$(LINUX_DIR)/drivers/net/wireless/intersil/prism54/prism54.ko@ge4.5
  AUTOLOAD:=$(call AutoProbe,prism54)
endef

define KernelPackage/net-prism54/description
 Kernel modules for Intersil Prism54 support
endef

$(eval $(call KernelPackage,net-prism54))


define KernelPackage/net-rtl8192su
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8192SU support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +kmod-usb-core +rtl8192su-firmware
  KCONFIG:=\
	CONFIG_STAGING=y \
	CONFIG_R8712U
  FILES:=$(LINUX_DIR)/drivers/staging/rtl8712/r8712u.ko
  AUTOLOAD:=$(call AutoProbe,r8712u)
endef

define KernelPackage/net-rtl8192su/description
 Kernel modules for RealTek RTL8712 and RTL81XXSU fullmac support.
endef

$(eval $(call KernelPackage,net-rtl8192su))
