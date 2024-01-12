# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Cezary Jackiewicz <cezary@eko.one.pll>

define KernelPackage/amd-xgbe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=AMD Ethernet on SoC support
  DEPENDS:=@PCI_SUPPORT @TARGET_x86_64 +kmod-lib-crc32c +kmod-ptp +kmod-libphy \
	+(LINUX_5_10||LINUX_5_15||LINUX_6_1):kmod-mdio-devres
  KCONFIG:=CONFIG_AMD_XGBE
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/amd/xgbe/amd-xgbe.ko
  AUTOLOAD:=$(call AutoLoad,35,amd-xgbe)
endef

define KernelPackage/amd-xgbe/description
 Kernel modules for AMD 10GbE Ethernet device on an AMD SoC.
endef

$(eval $(call KernelPackage,amd-xgbe))


define KernelPackage/sound-cs5535audio
  TITLE:=CS5535/CS5536 Audio Controller
  DEPENDS:=@TARGET_x86_geode +kmod-ac97
  KCONFIG:=CONFIG_SND_CS5535AUDIO
  FILES:=$(LINUX_DIR)/sound/pci/cs5535audio/snd-cs5535audio.ko
  AUTOLOAD:=$(call AutoLoad,36,snd-cs5535audio)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-cs5535audio/description
 Support for the integrated AC97 sound device on motherboards
 with AMD CS5535/CS5536 chipsets.
endef

$(eval $(call KernelPackage,sound-cs5535audio))

define KernelPackage/sp5100-tco
  SUBMENU:=$(OTHER_MENU)
  TITLE:=SP5100 Watchdog Support
  DEPENDS:=@TARGET_x86
  KCONFIG:=CONFIG_SP5100_TCO
  FILES:=$(LINUX_DIR)/drivers/watchdog/sp5100_tco.ko
  AUTOLOAD:=$(call AutoLoad,50,sp5100_tco,1)
endef

define KernelPackage/sp5100-tco/description
 Kernel module for the SP5100_TCO hardware watchdog.
endef

$(eval $(call KernelPackage,sp5100-tco))


define KernelPackage/pcengines-apuv2
  SUBMENU:=$(OTHER_MENU)
  TITLE:=PC Engines APUv2/3 front button and LEDs driver
  DEPENDS:=@TARGET_x86 +kmod-gpio-amd-fch +kmod-leds-gpio
  KCONFIG:=CONFIG_PCENGINES_APU2
  FILES:=$(LINUX_DIR)/drivers/platform/x86/pcengines-apuv2.ko
  AUTOLOAD:=$(call AutoLoad,60,pcengines-apuv2)
endef

define KernelPackage/pcengines-apuv2/description
  This driver provides support for the front button and LEDs on
  PC Engines APUv2/APUv3 board.
endef

$(eval $(call KernelPackage,pcengines-apuv2))
