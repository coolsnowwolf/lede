#
# Download realtek r8168 linux driver from official site:
# [https://www.realtek.com/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-pci-express-software]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8168
PKG_VERSION:=8.050.02
PKG_RELEASE:=$(AUTORELEAE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/BROBIRD/openwrt-r8168.git
PKG_SOURCE_DATE:=2022-05-24
PKG_SOURCE_VERSION:=e7995591e0cfb538542d5b60b11ddd41d95d2743
PKG_MIRROR_HASH:=6f5e423c852112ab6a083faa82ab0948cd28e5815c667dc176ae00b5bbaa6168

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8168
  TITLE:=Driver for Realtek r8168 chipsets
  SUBMENU:=Network Devices
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:= $(PKG_BUILD_DIR)/src/r8168.ko
  AUTOLOAD:=$(call AutoProbe,r8168)
  CONFLICTS:=kmod-r8169
endef

define Package/r8168/description
  This package contains a driver for Realtek r8168 chipsets.
endef

R8168_MAKEOPTS= -C $(PKG_BUILD_DIR)/src \
	PATH="$(TARGET_PATH)" \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	TARGET="$(HAL_TARGET)" \
	TOOLPREFIX="$(KERNEL_CROSS)" \
	TOOLPATH="$(KERNEL_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	KERNELDIR="$(LINUX_DIR)" \
	LDOPTS=" " \
	DOMULTI=1

define Build/Compile
	$(MAKE) $(R8168_MAKEOPTS) modules
endef

$(eval $(call KernelPackage,r8168))
