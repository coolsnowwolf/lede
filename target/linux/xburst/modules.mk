

SOUND_MENU:=Sound Support

define KernelPackage/sound-soc-jz4740
  SUBMENU:=$(SOUND_MENU)
  DEPENDS:=kmod-sound-soc-core @TARGET_xburst @BROKEN
  TITLE:=JZ4740 SoC sound support
  KCONFIG:=CONFIG_SND_JZ4740_SOC CONFIG_SND_JZ4740_SOC_I2S
  FILES:= \
	$(LINUX_DIR)/sound/soc/jz4740/snd-soc-jz4740.ko \
	$(LINUX_DIR)/sound/soc/jz4740/snd-soc-jz4740-i2s.ko
  AUTOLOAD:=$(call AutoLoad,60,snd-soc-jz4740 snd-soc-jz4740-i2s)
endef

define KernelPackage/sound-soc-jz4740-codec
  SUBMENU:=$(SOUND_MENU)
  DEPENDS:=kmod-sound-soc-core @TARGET_xburst @BROKEN
  TITLE:=JZ4740 SoC internal codec support
  KCONFIG:=CONFIG_SND_SOC_JZ4740_CODEC
  FILES:=$(LINUX_DIR)/sound/soc/codecs/snd-soc-jz4740-codec.ko
  AUTOLOAD:=$(call AutoLoad,60,snd-soc-jz4740-codec)
endef

define KernelPackage/sound-soc-xburst/default
  SUBMENU:=$(SOUND_MENU)
  DEPENDS:=kmod-sound-soc-jz4740 kmod-sound-soc-jz4740-codec @TARGET_xburst_$(if $(4),$(4),$(3)) @BROKEN
  TITLE:=$(1) sound support
  KCONFIG:=CONFIG_SND_JZ4740_SOC_$(2)
  FILES:=$(LINUX_DIR)/sound/soc/jz4740/snd-soc-$(3).ko
  AUTOLOAD:=$(call AutoLoad,65,snd-soc-$(3))
endef

define KernelPackage/sound-soc-qilb60
$(call KernelPackage/sound-soc-xburst/default,QI NanoNote,QI_LB60,qi-lb60,qi_lb60)
endef

$(eval $(call KernelPackage,sound-soc-jz4740))
$(eval $(call KernelPackage,sound-soc-jz4740-codec))
$(eval $(call KernelPackage,sound-soc-qilb60))
