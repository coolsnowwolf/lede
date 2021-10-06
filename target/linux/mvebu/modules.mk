define KernelPackage/linkstation-poweroff
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@TARGET_mvebu
  TITLE:=Buffalo LinkStation power off driver
  KCONFIG:= \
	CONFIG_POWER_RESET=y \
	CONFIG_POWER_RESET_LINKSTATION \
	CONFIG_POWER_RESET_QNAP=n

  FILES:=$(LINUX_DIR)/drivers/power/reset/linkstation-poweroff.ko
  AUTOLOAD:=$(call AutoLoad,31,linkstation-poweroff,1)
endef

define KernelPackage/linkstation-poweroff/description
  This driver supports turning off some Buffalo LinkStations by
  setting an output pin at the ethernet PHY to the correct state.
  It also makes the device compatible with the WoL function.
  Say Y here if you have a Buffalo LinkStation LS421D/E.
endef

$(eval $(call KernelPackage,linkstation-poweroff))
