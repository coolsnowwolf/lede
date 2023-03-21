#
# Download realtek r8152 linux driver from official site:
# [https://www.realtek.com/en/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-usb-3-0-software]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8152
PKG_VERSION:=2.16.3-4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/awesometic/realtek-r8152-dkms/tar.gz/$(PKG_VERSION)?
PKG_HASH:=a80563c6abb05f4aeab5be9d63b0497859878c86e0f72c49dd527de2adec7b68
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/realtek-$(PKG_NAME)-dkms-$(PKG_VERSION)

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-net-rtl8152-vendor
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  TITLE:=Realtek RTL8152/RTL8153/RTL8154/RTL8156 driver
  SUBMENU:=USB Support
  DEPENDS:=+kmod-usb-net
  FILES:=$(PKG_BUILD_DIR)/src/r8152.ko
  AUTOLOAD:=$(call AutoProbe,r8152)
  CONFLICTS:=kmod-usb-net-rtl8152
endef

define KernelPackage/usb-net-rtl8152-vendor/description
  Kernel module for Realtek RTL8152 USB Ethernet chipsets
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/src modules
endef

$(eval $(call KernelPackage,usb-net-rtl8152-vendor))
