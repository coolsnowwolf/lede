#Download realtek r8168 linux driver from official site [https://www.realtek.com/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-pci-express-software]
#Unpack source file
#Replace orginal Makefile with this file
#Put this source to 'package' folder of OpenWRT/LEDE SDK
#Build(make menuconfig, make defconfig, make)

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8168
PKG_VERSION:=8.048.03
PKG_RELEASE:=3

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8168
  TITLE:=Driver for Realtek r8168 chipsets
  SUBMENU:=Network Devices
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:= $(PKG_BUILD_DIR)/r8168.ko
  AUTOLOAD:=$(call AutoProbe,r8168)
endef

define Package/r8168/description
 This package contains a driver for Realtek r8168 chipsets.
endef

R8168_MAKEOPTS= -C $(PKG_BUILD_DIR) \
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

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) $(R8168_MAKEOPTS) modules
endef

$(eval $(call KernelPackage,r8168))
