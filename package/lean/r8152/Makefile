#
# Download realtek r8152 linux driver from official site:
# [https://www.realtek.com/Download/List?cate_id=585]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8152
PKG_VERSION:=2.20.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/wget/realtek-r8152-linux/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=045a1e1d20336d26b61b01d8561216029ff3405894707958b5670a7cd7fa75c1
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/realtek-$(PKG_NAME)-linux-$(PKG_VERSION)

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-net-rtl8152-vendor
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  TITLE:=Realtek RTL8152/RTL8153/RTL8154/RTL8156/RTL8157 driver
  SUBMENU:=USB Support
  DEPENDS:=+kmod-usb-net +LINUX_6_12:kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/r8152.ko
  AUTOLOAD:=$(call AutoProbe,r8152)
  CONFLICTS:=kmod-usb-net-rtl8152
endef

define KernelPackage/usb-net-rtl8152-vendor/description
  Kernel module for Realtek Realtek USB 1G / 2.5G / 5G / 10G Ethernet Chipsets
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,usb-net-rtl8152-vendor))
