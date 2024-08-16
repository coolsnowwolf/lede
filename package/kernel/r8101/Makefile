include $(TOPDIR)/rules.mk

PKG_NAME:=r8101
PKG_VERSION:=1.039.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8101/releases/download/$(PKG_VERSION)
PKG_HASH:=e64e1738e71d6717dd844bf771fea4691edae63e92d7d03bb5ad2ef08e56e72b

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8101
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8101 PCI Fast Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8101.ko
  AUTOLOAD:=$(call AutoProbe,r8101)
  PROVIDES:=kmod-r8169
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		modules
endef

$(eval $(call KernelPackage,r8101))
