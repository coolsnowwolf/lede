#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

define KernelPackage/rtc-sunxi
    SUBMENU:=$(OTHER_MENU)
    TITLE:=Sunxi SoC built-in RTC support
    DEPENDS:=@TARGET_sunxi
    $(call AddDepends/rtc)
    KCONFIG:= \
	CONFIG_RTC_CLASS=y \
	CONFIG_RTC_DRV_SUNXI=m
    FILES:=$(LINUX_DIR)/drivers/rtc/rtc-sunxi.ko
    AUTOLOAD:=$(call AutoLoad,50,rtc-sunxi)
endef

define KernelPackage/rtc-sunxi/description
 Support for the AllWinner sunXi SoC's onboard RTC
endef

$(eval $(call KernelPackage,rtc-sunxi))

define KernelPackage/sunxi-ir
    SUBMENU:=$(OTHER_MENU)
    TITLE:=Sunxi SoC built-in IR support (A20)
    DEPENDS:=@TARGET_sunxi +kmod-input-core
    $(call AddDepends/rtc)
    KCONFIG:= \
	CONFIG_MEDIA_SUPPORT=y \
	CONFIG_MEDIA_RC_SUPPORT=y \
	CONFIG_RC_DEVICES=y \
	CONFIG_IR_SUNXI
    FILES:=$(LINUX_DIR)/drivers/media/rc/sunxi-cir.ko
    AUTOLOAD:=$(call AutoLoad,50,sunxi-cir)
endef

define KernelPackage/sunxi-ir/description
 Support for the AllWinner sunXi SoC's onboard IR (A20)
endef

$(eval $(call KernelPackage,sunxi-ir))

define KernelPackage/ata-sunxi
    TITLE:=AllWinner sunXi AHCI SATA support
    SUBMENU:=$(BLOCK_MENU)
    DEPENDS:=@TARGET_sunxi +kmod-ata-ahci-platform +kmod-scsi-core
    KCONFIG:=CONFIG_AHCI_SUNXI
    FILES:=$(LINUX_DIR)/drivers/ata/ahci_sunxi.ko
    AUTOLOAD:=$(call AutoLoad,41,ahci_sunxi,1)
endef

define KernelPackage/ata-sunxi/description
 SATA support for the AllWinner sunXi SoC's onboard AHCI SATA
endef

$(eval $(call KernelPackage,ata-sunxi))

define KernelPackage/sun4i-emac
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=AllWinner EMAC Ethernet support
  DEPENDS:=@TARGET_sunxi +kmod-of-mdio +kmod-libphy
  KCONFIG:=CONFIG_SUN4I_EMAC
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/allwinner/sun4i-emac.ko
  AUTOLOAD:=$(call AutoProbe,sun4i-emac)
endef

$(eval $(call KernelPackage,sun4i-emac))


define KernelPackage/sound-soc-sunxi
  TITLE:=AllWinner built-in SoC sound support
  KCONFIG:=CONFIG_SND_SUN4I_CODEC
  FILES:=$(LINUX_DIR)/sound/soc/sunxi/sun4i-codec.ko
  AUTOLOAD:=$(call AutoLoad,65,sun4i-codec)
  DEPENDS:=@TARGET_sunxi +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-sunxi/description
  Kernel support for AllWinner built-in SoC audio
endef

$(eval $(call KernelPackage,sound-soc-sunxi))
