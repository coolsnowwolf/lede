include $(TOPDIR)/rules.mk

PKG_NAME:=yt6801
PKG_VERSION:=1.0.30
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=Lean <coolsnowwolf@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/yt6801
  SUBMENU:=Network Devices
  TITLE:=Motorcomm YT6801 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/yt6801.ko
  AUTOLOAD:=$(call AutoProbe,yt6801)
  KCONFIG:=
endef

define KernelPackage/yt6801/description
  Kernel module for Motorcomm YT6801 PCI Gigabit Ethernet controllers.
  This driver supports the YT6801 chipset from Motorcomm.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="-DFXGMAC_DEBUG" \
		modules
endef

$(eval $(call KernelPackage,yt6801))