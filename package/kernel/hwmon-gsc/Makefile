include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hwmon-gsc
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/hwmon-gsc
  SUBMENU:=Hardware Monitoring Support
  DEPENDS:=@TARGET_imx +kmod-hwmon-core +kmod-i2c-core
  TITLE:=Driver for the Gateworks System Controller
  AUTOLOAD:=$(call AutoLoad,60,gsc)
  FILES:=$(PKG_BUILD_DIR)/gsc.ko
endef

define KernelPackage/hwmon-gsc/description
  Kernel module for the Gateworks System Controller chips.
endef

define Build/Compile
	$(KERNEL_MAKE) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,hwmon-gsc))
