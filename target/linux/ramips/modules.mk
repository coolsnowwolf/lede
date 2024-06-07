# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2016 OpenWrt.org

OTHER_MENU:=Other modules

define KernelPackage/pwm-mediatek-ramips
  SUBMENU:=Other modules
  TITLE:=MT7628 PWM
  DEPENDS:=@(TARGET_ramips_mt76x8)
  KCONFIG:= \
	CONFIG_PWM=y \
	CONFIG_PWM_MEDIATEK_RAMIPS \
	CONFIG_PWM_SYSFS=y
  FILES:= \
	$(LINUX_DIR)/drivers/pwm/pwm-mediatek-ramips.ko
  AUTOLOAD:=$(call AutoProbe,pwm-mediatek-ramips)
endef

define KernelPackage/pwm-mediatek-ramips/description
  Kernel modules for MediaTek Pulse Width Modulator
endef

$(eval $(call KernelPackage,pwm-mediatek-ramips))

define KernelPackage/sdhci-mt7620
  SUBMENU:=Other modules
  TITLE:=MT7620 SDCI
  DEPENDS:=@(TARGET_ramips_mt7620||TARGET_ramips_mt76x8||TARGET_ramips_mt7621) +kmod-mmc
  KCONFIG:= \
	CONFIG_MTK_MMC \
	CONFIG_MTK_AEE_KDUMP=n \
	CONFIG_MTK_MMC_CD_POLL=n
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/mtk-mmc/mtk_sd.ko
  AUTOLOAD:=$(call AutoProbe,mtk_sd,1)
endef

$(eval $(call KernelPackage,sdhci-mt7620))

I2C_RALINK_MODULES:= \
  CONFIG_I2C_RALINK:drivers/i2c/busses/i2c-ralink

define KernelPackage/i2c-ralink
  $(call i2c_defaults,$(I2C_RALINK_MODULES),59)
  TITLE:=Ralink I2C Controller
  DEPENDS:=+kmod-i2c-core @TARGET_ramips \
	@!(TARGET_ramips_mt7621||TARGET_ramips_mt76x8)
endef

define KernelPackage/i2c-ralink/description
 Kernel modules for enable ralink i2c controller.
endef

$(eval $(call KernelPackage,i2c-ralink))


I2C_MT7621_MODULES:= \
  CONFIG_I2C_MT7621:drivers/i2c/busses/i2c-mt7621

define KernelPackage/i2c-mt7628
  $(call i2c_defaults,$(I2C_MT7621_MODULES),59)
  TITLE:=MT7628/88 I2C Controller
  DEPENDS:=+kmod-i2c-core \
	@(TARGET_ramips_mt76x8)
endef

define KernelPackage/i2c-mt7628/description
 Kernel modules for enable mt7621 i2c controller.
endef

$(eval $(call KernelPackage,i2c-mt7628))

define KernelPackage/dma-ralink
  SUBMENU:=Other modules
  TITLE:=Ralink GDMA Engine
  DEPENDS:=@TARGET_ramips
  KCONFIG:= \
	CONFIG_DMADEVICES=y \
	CONFIG_DW_DMAC_PCI=n \
	CONFIG_DMA_RALINK
  FILES:= \
	$(LINUX_DIR)/drivers/dma/virt-dma.ko \
	$(LINUX_DIR)/drivers/staging/ralink-gdma/ralink-gdma.ko
  AUTOLOAD:=$(call AutoLoad,52,ralink-gdma)
endef

define KernelPackage/dma-ralink/description
 Kernel modules for enable ralink dma engine.
endef

$(eval $(call KernelPackage,dma-ralink))

define KernelPackage/hsdma-mtk
  SUBMENU:=Other modules
  TITLE:=MediaTek HSDMA Engine
  DEPENDS:=@TARGET_ramips @TARGET_ramips_mt7621
  KCONFIG:= \
	CONFIG_DMADEVICES=y \
	CONFIG_DW_DMAC_PCI=n \
	CONFIG_MTK_HSDMA
  FILES:= \
	$(LINUX_DIR)/drivers/dma/virt-dma.ko \
	$(LINUX_DIR)/drivers/staging/mt7621-dma/hsdma-mt7621.ko
  AUTOLOAD:=$(call AutoLoad,53,hsdma-mt7621)
endef

define KernelPackage/hsdma-mtk/description
 Kernel modules for enable MediaTek hsdma engine.
endef

$(eval $(call KernelPackage,hsdma-mtk))

define KernelPackage/sound-mt7620
  TITLE:=MT7620 PCM/I2S Alsa Driver
  DEPENDS:=@TARGET_ramips @!TARGET_ramips_rt288x +kmod-dma-ralink \
	+kmod-sound-soc-core +kmod-sound-soc-wm8960
  KCONFIG:= \
	CONFIG_SND_RALINK_SOC_I2S \
	CONFIG_SND_SIMPLE_CARD \
	CONFIG_SND_SIMPLE_CARD_UTILS
  FILES:= \
	$(LINUX_DIR)/sound/soc/ralink/snd-soc-ralink-i2s.ko \
	$(LINUX_DIR)/sound/soc/generic/snd-soc-simple-card.ko \
	$(LINUX_DIR)/sound/soc/generic/snd-soc-simple-card-utils.ko
  AUTOLOAD:=$(call AutoLoad,90,snd-soc-ralink-i2s snd-soc-simple-card)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-mt7620/description
 Alsa modules for ralink i2s controller.
endef

$(eval $(call KernelPackage,sound-mt7620))

define KernelPackage/mtk-hnat
  SUBMENU:=Network Devices
  TITLE:=MediaTek MT762x HW NAT driver
  DEPENDS:=@TARGET_ramips @TARGET_ramips_mt7621 +kmod-nf-flow
  KCONFIG:= \
	CONFIG_BRIDGE_NETFILTER=y \
	CONFIG_NET_MEDIATEK_HNAT \
	CONFIG_NETFILTER_FAMILY_BRIDGE=y
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/mtk/mtk_hnat/mtkhnat.ko
  AUTOLOAD:=$(call AutoLoad,55,mtkhnat)
endef

$(eval $(call KernelPackage,mtk-hnat))
