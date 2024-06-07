define KernelPackage/hwmon-phytium
  SUBMENU:=$(HWMON_MENU)
  TITLE:=Phytium monitoring support
  KCONFIG:=CONFIG_SENSORS_PHYTIUM
  FILES:=$(LINUX_DIR)/drivers/hwmon/tacho-phytium.ko
  AUTOLOAD:=$(call AutoLoad,50,tacho-phytium)
  $(call AddDepends/hwmon,@TARGET_phytium)
endef

define KernelPackage/hwmon-phytium/description
  Hwmon driver for Phytium tachometer.
endef

$(eval $(call KernelPackage,hwmon-phytium))

define KernelPackage/drm-phytium
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=DRM Support for Phytium Graphics Card
  DEPENDS:=@TARGET_phytium @DISPLAY_SUPPORT +kmod-drm +kmod-drm-kms-helper \
	+(LINUX_5_10||LINUX_6_1||LINUX_6_3||LINUX_6_4||LINUX_6_5||LINUX_6_6):kmod-acpi-video
  KCONFIG:=CONFIG_DRM_PHYTIUM
  FILES:=$(LINUX_DIR)/drivers/gpu/drm/phytium/phytium-dc-drm.ko
  AUTOLOAD:=$(call AutoProbe,phytium-dc-drm)
endef

define KernelPackage/drm-phytium/description
  Direct Rendering Manager (DRM) support for Phytium Graphics Card
endef

$(eval $(call KernelPackage,drm-phytium))
