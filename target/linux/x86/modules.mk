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

define KernelPackage/sp5100_tco
  SUBMENU:=$(OTHER_MENU)
  TITLE:=SP5100 Watchdog Support
  DEPENDS:=@TARGET_x86
  KCONFIG:=CONFIG_SP5100_TCO
  FILES:=$(LINUX_DIR)/drivers/watchdog/sp5100_tco.ko
  AUTOLOAD:=$(call AutoLoad,50,sp5100_tco,1)
endef

define KernelPackage/sp5100_tco/description
 Kernel module for the SP5100_TCO hardware watchdog.
endef

$(eval $(call KernelPackage,sp5100_tco))
