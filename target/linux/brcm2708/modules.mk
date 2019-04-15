#
# Copyright (C) 2012-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/drm-vc4
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Broadcom VC4 Graphics
  DEPENDS:= \
	@TARGET_brcm2708 +kmod-drm \
	+LINUX_4_14:kmod-sound-core \
	+LINUX_4_14:kmod-sound-soc-core
  KCONFIG:= \
	CONFIG_DRM_VC4 \
	CONFIG_DRM_VC4_HDMI_CEC=n
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/vc4/vc4.ko \
	$(LINUX_DIR)/drivers/gpu/drm/drm_kms_helper.ko
  AUTOLOAD:=$(call AutoProbe,vc4)
endef

define KernelPackage/drm-vc4/description
  Direct Rendering Manager (DRM) support for Broadcom VideoCore IV GPU
  used in BCM2835, BCM2836 and BCM2837 SoCs (e.g. Raspberry Pi).
endef

$(eval $(call KernelPackage,drm-vc4))


define KernelPackage/hwmon-rpi-poe-fan
  SUBMENU:=$(HWMON_MENU)
  TITLE:=Raspberry Pi PoE HAT fan
  DEPENDS:=@TARGET_brcm2708 @LINUX_4_14 +kmod-hwmon-core
  KCONFIG:=CONFIG_SENSORS_RPI_POE_FAN
  FILES:=$(LINUX_DIR)/drivers/hwmon/rpi-poe-fan.ko
  AUTOLOAD:=$(call AutoProbe,rpi-poe-fan)
endef

define KernelPackage/hwmon-rpi-poe-fan/description
  Raspberry Pi PoE HAT fan driver
endef

$(eval $(call KernelPackage,hwmon-rpi-poe-fan))


define KernelPackage/sound-arm-bcm2835
  TITLE:=BCM2835 ALSA driver
  KCONFIG:= \
	CONFIG_SND_ARM=y \
	CONFIG_SND_BCM2835 \
	CONFIG_SND_ARMAACI=n
  FILES:= \
	$(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-audio/snd-bcm2835.ko@ge4.12 \
	$(LINUX_DIR)/sound/arm/snd-bcm2835.ko@lt4.12
  AUTOLOAD:=$(call AutoLoad,68,snd-bcm2835)
  DEPENDS:=@TARGET_brcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-arm-bcm2835/description
  This package contains the BCM2835 ALSA pcm card driver
endef

$(eval $(call KernelPackage,sound-arm-bcm2835))


define KernelPackage/sound-soc-bcm2835-i2s
  TITLE:=SoC Audio support for the Broadcom 2835 I2S module
  KCONFIG:= \
	CONFIG_SND_BCM2835_SOC_I2S \
	CONFIG_SND_SOC_DMAENGINE_PCM=y \
	CONFIG_SND_SOC_GENERIC_DMAENGINE_PCM=y
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-bcm2835-i2s.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-bcm2835-i2s)
  DEPENDS:=@TARGET_brcm2708 +kmod-regmap-mmio +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-bcm2835-i2s/description
  This package contains support for codecs attached to the Broadcom 2835 I2S interface
endef

$(eval $(call KernelPackage,sound-soc-bcm2835-i2s))


define KernelPackage/sound-soc-3dlab-nano-player
  TITLE:=Support for 3Dlab Nano Player
  KCONFIG:= CONFIG_SND_BCM2708_SOC_3DLAB_NANO_PLAYER
  FILES:=$(LINUX_DIR)/sound/soc/bcm/snd-soc-3dlab-nano-player.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-3dlab-nano-player)
  DEPENDS:= \
	@LINUX_4_14 \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-regmap-i2c
endef

define KernelPackage/sound-soc-3dlab-nano-player/description
  This package contains support for 3Dlab Nano Player
endef

$(eval $(call KernelPackage,sound-soc-3dlab-nano-player))


define KernelPackage/sound-soc-adau1977-adc
  TITLE:=Support for ADAU1977 ADC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_ADAU1977_ADC \
	CONFIG_SND_SOC_ADAU1977 \
	CONFIG_SND_SOC_ADAU1977_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-adau1977-adc.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-adau1977.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-adau1977-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-adau1977 snd-soc-adau1977-i2c \
	snd-soc-adau1977-adc)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-adau1977-adc/description
  This package contains support for ADAU1977 ADC
endef

$(eval $(call KernelPackage,sound-soc-adau1977-adc))


define KernelPackage/sound-soc-allo-boss-dac
  TITLE:=Support for Allo Boss DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_ALLO_BOSS_DAC \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-boss-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
	snd-soc-allo-boss-dac)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-boss-dac/description
  This package contains support for Allo Boss DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-boss-dac))


define KernelPackage/sound-soc-allo-digione
  TITLE:=Support for Allo Piano DigiOne
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_ALLO_DIGIONE \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-digione.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
	snd-soc-allo-digione)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-digione/description
  This package contains support for Allo DigiOne
endef

$(eval $(call KernelPackage,sound-soc-allo-digione))


define KernelPackage/sound-soc-allo-piano-dac
  TITLE:=Support for Allo Piano DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_ALLO_PIANO_DAC \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-piano-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
	snd-soc-allo-piano-dac)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-piano-dac/description
  This package contains support for Allo Piano DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-piano-dac))


define KernelPackage/sound-soc-allo-piano-dac-plus
  TITLE:=Support for Allo Piano DAC Plus
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_ALLO_PIANO_DAC_PLUS \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-piano-dac-plus.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
	snd-soc-allo-piano-dac-plus)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-piano-dac-plus/description
  This package contains support for Allo Piano DAC Plus
endef

$(eval $(call KernelPackage,sound-soc-allo-piano-dac-plus))


define KernelPackage/sound-soc-allo-katana-codec
  TITLE:=Support for Allo Katana DAC
  KCONFIG:= \
	CONFIG_SND_AUDIO_GRAPH_CARD \
	CONFIG_SND_BCM2708_SOC_ALLO_KATANA_DAC \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C \
	CONFIG_SND_SIMPLE_CARD_UTILS
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-katana-codec.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
	snd-soc-allo-katana-codec)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	+kmod-regmap-core \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-katana-codec/description
  This package contains support for Allo Katana DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-katana-codec))


define KernelPackage/sound-soc-audioinjector-octo-soundcard
  TITLE:=Support for AudioInjector Octo soundcard
  KCONFIG:= \
	CONFIG_SND_AUDIOINJECTOR_OCTO_SOUNDCARD \
	CONFIG_SND_SOC_CS42XX8 \
	CONFIG_SND_SOC_CS42XX8_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-audioinjector-octo-soundcard.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-cs42xx8.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-cs42xx8-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc- \
	snd-soc-audioinjector-octo-soundcard)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-audioinjector-octo-soundcard/description
  This package contains support for AudioInjector Octo soundcard
endef

$(eval $(call KernelPackage,sound-soc-audioinjector-octo-soundcard))


define KernelPackage/sound-soc-audioinjector-pi-soundcard
  TITLE:=Support for AudioInjector Pi soundcard
  KCONFIG:= \
        CONFIG_SND_AUDIOINJECTOR_PI_SOUNDCARD \
        CONFIG_SND_SOC_WM8731
  FILES:= \
        $(LINUX_DIR)/sound/soc/bcm/snd-soc-audioinjector-pi-soundcard.ko \
        $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8731.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8731 \
	snd-soc-audioinjector-pi-soundcard)
  DEPENDS:= \
        kmod-sound-soc-bcm2835-i2s \
        +kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-audioinjector-pi-soundcard/description
  This package contains support for AudioInjector Pi soundcard
endef

$(eval $(call KernelPackage,sound-soc-audioinjector-pi-soundcard))

define KernelPackage/sound-soc-digidac1-soundcard
  TITLE:=Support for RRA DigiDAC1
  KCONFIG:= \
	CONFIG_SND_DIGIDAC1_SOUNDCARD \
	CONFIG_SND_SOC_WM8741 \
	CONFIG_SND_SOC_WM8804 \
	CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-digidac1-soundcard.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8741.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8741 \
	snd-soc-wm8804 snd-soc-wm8804-i2c \
	snd-soc-digidac1-soundcard)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-digidac1-soundcard/description
  This package contains support for RRA DigiDAC1
endef

$(eval $(call KernelPackage,sound-soc-digidac1-soundcard))

define KernelPackage/sound-soc-dionaudio-loco
  TITLE:=Support for Dion Audio LOCO DAC-AMP
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_DIONAUDIO_LOCO \
	CONFIG_SND_SOC_PCM5102A
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-dionaudio-loco.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm5102a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm5102a \
	snd-soc-dionaudio-loco)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-dionaudio-loco/description
  This package contains support for Dion Audio LOCO DAC-AMP
endef

$(eval $(call KernelPackage,sound-soc-dionaudio-loco))

define KernelPackage/sound-soc-dionaudio-loco-v2
  TITLE:=Support for Dion Audio LOCO-V2 DAC-AMP
  KCONFIG:= \
        CONFIG_SND_BCM2708_SOC_DIONAUDIO_LOCO_V2 \
        CONFIG_SND_SOC_PCM512x \
        CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
        $(LINUX_DIR)/sound/soc/bcm/snd-soc-dionaudio-loco.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x snd-soc-pcm512x-i2c \
        snd-soc-dionaudio-loco)
  DEPENDS:= \
        kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-dionaudio-loco-v2/description
  This package contains support for Dion Audio LOCO-V2 DAC-AMP
endef

$(eval $(call KernelPackage,sound-soc-dionaudio-loco-v2))

define KernelPackage/sound-soc-fe-pi
  TITLE:=Support for Fe-Pi Audio Sound Card
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_FE_PI_AUDIO \
	CONFIG_SND_SOC_SGTL5000
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-fe-pi-audio.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-sgtl5000.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-sgtl5000 \
	snd-soc-fe-pi-audio)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-fe-pi/description
  This package contains support for Fe-Pi Audio Sound Card
endef

$(eval $(call KernelPackage,sound-soc-fe-pi))


define KernelPackage/sound-soc-googlevoicehat
  TITLE:=Support for Google VoiceHAT Sound Card
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_GOOGLEVOICEHAT_SOUNDCARD \
	CONFIG_SND_SOC_VOICEHAT
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-googlevoicehat-codec.ko \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-googlevoicehat-soundcard.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-googlevoicehat-codec \
	snd-soc-googlevoicehat-soundcard)
  DEPENDS:= \
	@LINUX_4_14 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-googlevoicehat/description
  This package contains support for Google VoiceHAT Sound Card
endef

$(eval $(call KernelPackage,sound-soc-googlevoicehat))


define KernelPackage/sound-soc-hifiberry-dac
  TITLE:=Support for HifiBerry DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_HIFIBERRY_DAC \
	CONFIG_SND_SOC_PCM5102A
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm5102a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm5102a snd-soc-hifiberry-dac)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dac/description
  This package contains support for HifiBerry DAC
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dac))

define KernelPackage/sound-soc-hifiberry-dacplus
  TITLE:=Support for HifiBerry DAC+ / DAC+ Pro
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUS \
	CONFIG_SND_SOC_PCM512x
  FILES:= \
	$(LINUX_DIR)/drivers/clk/clk-hifiberry-dacpro.ko \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplus.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko
  AUTOLOAD:=$(call AutoLoad,68,clk-hifiberry-dacpro snd-soc-pcm512x \
	snd-soc-hifiberry-dacplus)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplus/description
  This package contains support for HifiBerry DAC+ / DAC+ Pro
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplus))

define KernelPackage/sound-soc-hifiberry-digi
  TITLE:=Support for HifiBerry Digi / Digi+ / Digi+ Pro
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_HIFIBERRY_DIGI \
	CONFIG_SND_SOC_WM8804
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-digi.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804 snd-soc-hifiberry-digi)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-digi/description
  This package contains support for HifiBerry Digi
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-digi))

define KernelPackage/sound-soc-hifiberry-amp
  TITLE:=Support for HifiBerry Amp
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_HIFIBERRY_AMP \
	CONFIG_SND_SOC_TAS5713
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-amp.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-tas5713.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-tas5713 snd-soc-hifiberry-amp)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-amp/description
  This package contains support for HifiBerry Amp
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-amp))

define KernelPackage/sound-soc-iqaudio-dac
  TITLE:=Support for IQaudIO-DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_IQAUDIO_DAC \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-iqaudio-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x snd-soc-pcm512x-i2c \
	snd-soc-iqaudio-dac)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-iqaudio-dac/description
  This package contains support for IQaudIO-DAC
endef

$(eval $(call KernelPackage,sound-soc-iqaudio-dac))

define KernelPackage/sound-soc-iqaudio-digi
  TITLE:=Support for IQaudIO-DIGI
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_IQAUDIO_DIGI \
	CONFIG_SND_SOC_WM8804 \
	CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-iqaudio-digi.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804 snd-soc-wm8804-i2c \
	snd-soc-iqaudio-digi)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-iqaudio-digi/description
  This package contains support for IQaudIO-DIGI
endef

$(eval $(call KernelPackage,sound-soc-iqaudio-digi))

define KernelPackage/sound-soc-justboom-dac
  TITLE:=Support for JustBoom DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_JUSTBOOM_DAC \
	CONFIG_SND_SOC_PCM512x
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-justboom-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x snd-soc-justboom-dac)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-justboom-dac/description
  This package contains support for JustBoom DAC
endef

$(eval $(call KernelPackage,sound-soc-justboom-dac))

define KernelPackage/sound-soc-justboom-digi
  TITLE:=Support for JustBoom Digi
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_JUSTBOOM_DIGI \
	CONFIG_SND_SOC_WM8804
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-justboom-digi.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804 snd-soc-justboom-digi)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-justboom-digi/description
  This package contains support for JustBoom Digi
endef

$(eval $(call KernelPackage,sound-soc-justboom-digi))

define KernelPackage/sound-soc-pisound
  TITLE:=Support for Blokas Labs PiSound
  KCONFIG:= \
	CONFIG_SND_PISOUND \
	CONFIG_SND_SOC_PCM5102A
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-pisound.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm5102a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm5102a snd-soc-pisound)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-pisound/description
  This package contains support for Blokas Labs PiSound
endef

$(eval $(call KernelPackage,sound-soc-pisound))

define KernelPackage/sound-soc-raspidac3
  TITLE:=Support for RaspiDAC Rev.3x
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_RASPIDAC3 \
	CONFIG_SND_SOC_PCM512x \
	CONFIG_SND_SOC_PCM512x_I2C \
	CONFIG_SND_SOC_TPA6130A2
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-raspidac3.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-tpa6130a2.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x snd-soc-pcm512x-i2c \
	snd-soc-tpa6130a2 snd-soc-raspidac3)
  DEPENDS:= \
	@LINUX_4_9 \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-raspidac3/description
  This package contains support for RaspiDAC Rev.3x
endef

$(eval $(call KernelPackage,sound-soc-raspidac3))


define KernelPackage/sound-soc-rpi-cirrus
  TITLE:=Support for Cirrus Logic Audio Card
  KCONFIG:= \
	CONFIG_GPIO_ARIZONA \
	CONFIG_INPUT_ARIZONA_HAPTICS=n \
	CONFIG_MFD_ARIZONA=y \
	CONFIG_MFD_ARIZONA_I2C \
	CONFIG_MFD_CS47L24=n \
	CONFIG_MFD_WM5102=n \
	CONFIG_MFD_WM5110=n \
	CONFIG_MFD_WM8997=n \
	CONFIG_MFD_WM8998=n \
	CONFIG_REGULATOR_ARIZONA \
	CONFIG_REGULATOR_ARIZONA_LDO1 \
	CONFIG_REGULATOR_ARIZONA_MICSUPP \
	CONFIG_SND_BCM2708_SOC_RPI_CIRRUS \
	CONFIG_SND_SOC_ARIZONA \
	CONFIG_SND_SOC_WM5102 \
	CONFIG_SND_SOC_WM8804 \
	CONFIG_SND_SOC_WM_ADSP
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-cirrus.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-arizona.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm-adsp.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm5102.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm1794a snd-soc-rpi-cirrus)
  DEPENDS:= \
	@LINUX_4_14 \
	+kmod-i2c-bcm2708 \
	kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-cirrus/description
  This package contains support for RPi-Cirrus
endef

$(eval $(call KernelPackage,sound-soc-rpi-cirrus))


define KernelPackage/sound-soc-rpi-dac
  TITLE:=Support for RPi-DAC
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_RPI_DAC \
	CONFIG_SND_SOC_PCM1794A
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-dac.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm1794a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm1794a snd-soc-rpi-dac)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-dac/description
  This package contains support for RPi-DAC
endef

$(eval $(call KernelPackage,sound-soc-rpi-dac))

define KernelPackage/sound-soc-rpi-proto
  TITLE:=Support for RPi-PROTO
  KCONFIG:= \
	CONFIG_SND_BCM2708_SOC_RPI_PROTO \
	CONFIG_SND_SOC_WM8731
  FILES:= \
	$(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-proto.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8731.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8731 snd-soc-rpi-proto)
  DEPENDS:= \
	kmod-sound-soc-bcm2835-i2s \
	+kmod-i2c-bcm2708
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-proto/description
  This package contains support for RPi-PROTO
endef

$(eval $(call KernelPackage,sound-soc-rpi-proto))


define KernelPackage/random-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 HW Random Number Generator
  KCONFIG:=CONFIG_HW_RANDOM_BCM2835
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/bcm2835-rng.ko
  AUTOLOAD:=$(call AutoLoad,11,bcm2835-rng)
  DEPENDS:=@TARGET_brcm2708 +kmod-random-core
endef

define KernelPackage/random-bcm2835/description
  This package contains the Broadcom 2835 HW random number generator driver
endef

$(eval $(call KernelPackage,random-bcm2835))


define KernelPackage/smi-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 SMI driver
  KCONFIG:=CONFIG_BCM2835_SMI
  FILES:=$(LINUX_DIR)/drivers/misc/bcm2835_smi.ko
  AUTOLOAD:=$(call AutoLoad,20,bcm2835_smi)
  DEPENDS:=@TARGET_brcm2708
endef

define KernelPackage/smi-bcm2835/description
  This package contains the Character device driver for Broadcom Secondary
  Memory Interface
endef

$(eval $(call KernelPackage,smi-bcm2835))

define KernelPackage/smi-bcm2835-dev
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 SMI device driver
  KCONFIG:=CONFIG_BCM2835_SMI_DEV
  FILES:=$(LINUX_DIR)/drivers/char/broadcom/bcm2835_smi_dev.ko
  AUTOLOAD:=$(call AutoLoad,21,bcm2835_smi_dev)
  DEPENDS:=@TARGET_brcm2708 +kmod-smi-bcm2835
endef

define KernelPackage/smi-bcm2835-dev/description
  This driver provides a character device interface (ioctl + read/write) to
  Broadcom's Secondary Memory interface. The low-level functionality is provided
  by the SMI driver itself.
endef

$(eval $(call KernelPackage,smi-bcm2835-dev))


define KernelPackage/spi-bcm2835
  SUBMENU:=$(SPI_MENU)
  TITLE:=BCM2835 SPI controller driver
  KCONFIG:=\
    CONFIG_SPI=y \
    CONFIG_SPI_BCM2835 \
    CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/spi/spi-bcm2835.ko
  AUTOLOAD:=$(call AutoLoad,89,spi-bcm2835)
  DEPENDS:=@TARGET_brcm2708
endef

define KernelPackage/spi-bcm2835/description
  This package contains the Broadcom 2835 SPI master controller driver
endef

$(eval $(call KernelPackage,spi-bcm2835))

define KernelPackage/spi-bcm2835-aux
  SUBMENU:=$(SPI_MENU)
  TITLE:=BCM2835 Aux SPI controller driver
  KCONFIG:=\
    CONFIG_SPI=y \
    CONFIG_SPI_BCM2835AUX \
    CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/spi/spi-bcm2835aux.ko
  AUTOLOAD:=$(call AutoLoad,89,spi-bcm2835aux)
  DEPENDS:=@TARGET_brcm2708
endef

define KernelPackage/spi-bcm2835-aux/description
  This package contains the Broadcom 2835 Aux SPI master controller driver
endef

$(eval $(call KernelPackage,spi-bcm2835-aux))


define KernelPackage/hwmon-bcm2835
  TITLE:=BCM2835 HWMON driver
  KCONFIG:=CONFIG_SENSORS_BCM2835
  FILES:=$(LINUX_DIR)/drivers/hwmon/bcm2835-hwmon.ko
  AUTOLOAD:=$(call AutoLoad,60,bcm2835-hwmon)
  $(call AddDepends/hwmon,@TARGET_brcm2708)
endef

define KernelPackage/hwmon-bcm2835/description
  Kernel module for BCM2835 thermal monitor chip
endef

$(eval $(call KernelPackage,hwmon-bcm2835))


I2C_BCM2708_MODULES:=\
  CONFIG_I2C_BCM2708:drivers/i2c/busses/i2c-bcm2708

define KernelPackage/i2c-bcm2708
  $(call i2c_defaults,$(I2C_BCM2708_MODULES),59)
  TITLE:=Broadcom BCM2708 I2C master controller driver
  KCONFIG+= \
	CONFIG_I2C_BCM2708_BAUDRATE=100000
  DEPENDS:=@TARGET_brcm2708 +kmod-i2c-core
endef

define KernelPackage/i2c-bcm2708/description
  This package contains the Broadcom 2708 I2C master controller driver
endef

$(eval $(call KernelPackage,i2c-bcm2708))

I2C_BCM2835_MODULES:=\
  CONFIG_I2C_BCM2835:drivers/i2c/busses/i2c-bcm2835

define KernelPackage/i2c-bcm2835
  $(call i2c_defaults,$(I2C_BCM2835_MODULES),59)
  TITLE:=Broadcom BCM2835 I2C master controller driver
  DEPENDS:=@TARGET_brcm2708 +kmod-i2c-core
endef

define KernelPackage/i2c-bcm2835/description
  This package contains the Broadcom 2835 I2C master controller driver
endef

$(eval $(call KernelPackage,i2c-bcm2835))


define KernelPackage/video-bcm2835
  TITLE:=Broadcom BCM2835 camera interface driver
  KCONFIG:= \
	CONFIG_VIDEO_BCM2835$(if $(CONFIG_LINUX_4_9),=y) \
	CONFIG_VIDEO_BCM2835_MMAL
  FILES:= \
	$(LINUX_DIR)/drivers/media/platform/bcm2835/bcm2835-v4l2.ko@lt4.12 \
	$(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-camera/bcm2835-v4l2.ko@ge4.12
  AUTOLOAD:=$(call AutoLoad,65,bcm2835-v4l2)
  $(call AddDepends/video,@TARGET_brcm2708 +kmod-video-videobuf2)
endef

define KernelPackage/video-bcm2835/description
  This is a V4L2 driver for the Broadcom 2835 MMAL camera host interface
endef

$(eval $(call KernelPackage,video-bcm2835))
