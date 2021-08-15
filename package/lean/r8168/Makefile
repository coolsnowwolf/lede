#
# Download realtek r8168 linux driver from official site:
# [https://www.realtek.com/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-pci-express-software]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8168
PKG_VERSION:=8.049.01
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/BROBIRD/openwrt-r8168.git
PKG_SOURCE_VERSION:=66fdc44a1f69c4fda18e6f922a234cbaa933ede6
PKG_MIRROR_HASH:=a480cf9c70bbc3ee5d9b967e5006ff4fc9bba2bf38638fb2b7567f10b6f41bf3

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
