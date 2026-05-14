#
# Download realtek r8152 linux driver from official site:
# [https://www.realtek.com/Download/List?cate_id=585]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8152
PKG_VERSION:=2.21.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/wget/realtek-r8152-linux/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=05041e46b0edb75b53bbdac640fc027b1eac22cf493e76c3018ab3b5c1dc1df8
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/realtek-$(PKG_NAME)-linux-$(PKG_VERSION)

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8152
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL815X USB Ethernet driver
  DEPENDS:=+kmod-usb-net +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/r8152.ko
  AUTOLOAD:=$(call AutoProbe,r8152)
  CONFLICTS:=kmod-usb-net-rtl8152
endef

define KernelPackage/r8152/description
  Kernel module for Realtek USB Ethernet Chipsets
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,r8152))
