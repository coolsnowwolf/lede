include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ubootenv-nvram
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ubootenv-nvram
  SUBMENU:=Other modules
  TITLE:=NVRAM environment for uboot-envtools
  FILES:=$(PKG_BUILD_DIR)/ubootenv-nvram.ko
  AUTOLOAD:=$(call AutoLoad,30,ubootenv-nvram,1)
  KCONFIG:=
endef

define KernelPackage/ubootenv-nvram/description
  Support vendor modified U-Boot storing the environment
  in RAM.  This driver exports the environment memory
  region as a misc device named "ubootenv", pretending
  it is a NOR mtd device to let existing userspace tools
  work without modifications.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,ubootenv-nvram))
