include $(TOPDIR)/rules.mk

PKG_NAME:=r8168
PKG_VERSION:=8.054.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8168/releases/download/$(PKG_VERSION)
PKG_HASH:=5480120cf823e991e8cbd325118c1ec0c57d8f42760ba1a7334bd07d291d235d

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8168
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8168 PCI Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8168.ko
  AUTOLOAD:=$(call AutoProbe,r8168)
  PROVIDES:=kmod-r8169
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		modules
endef

$(eval $(call KernelPackage,r8168))
