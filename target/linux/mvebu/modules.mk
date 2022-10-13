define KernelPackage/linkstation-poweroff
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@TARGET_mvebu
  TITLE:=Buffalo LinkStation power off driver
  KCONFIG:=CONFIG_POWER_RESET_LINKSTATION
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


define KernelPackage/leds-turris-omnia
   SUBMENU:=$(LEDS_MENU)
   TITLE:=LED support for CZ.NIC's Turris Omnia
   DEPENDS:=@TARGET_mvebu_cortexa9
   KCONFIG:=CONFIG_LEDS_TURRIS_OMNIA
   FILES:=$(LINUX_DIR)/drivers/leds/leds-turris-omnia.ko
   AUTOLOAD:=$(call AutoLoad,60,leds-turris-omnia,1)
endef

define KernelPackage/leds-turris-omnia/description
   This option enables support for the LEDs of the CZ.NIC Turris Omnia.
endef

$(eval $(call KernelPackage,leds-turris-omnia))
