#
# Copyright (C) 2017 Cezary Jackiewicz <cezary@eko.one.pll>
#
# This is free software, licensed under the GNU General Public License v2.
#

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
