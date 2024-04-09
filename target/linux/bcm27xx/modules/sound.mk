# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/sound-arm-bcm2835
  TITLE:=BCM2835 ALSA driver
  KCONFIG:= \
    CONFIG_SND_ARM=y \
    CONFIG_SND_BCM2835 \
    CONFIG_SND_ARMAACI=n
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-audio/snd-bcm2835.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-bcm2835)
  DEPENDS:=@TARGET_bcm27xx
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
  DEPENDS:=@TARGET_bcm27xx +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-bcm2835-i2s/description
  This package contains support for codecs attached to the Broadcom 2835 I2S interface
endef

$(eval $(call KernelPackage,sound-soc-bcm2835-i2s))


define KernelPackage/sound-soc-rpi-simple-soundcard
  TITLE:=Support for Raspberry Pi simple soundcards
  KCONFIG:= \
    CONFIG_SND_RPI_SIMPLE_SOUNDCARD
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-simple-soundcard.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-rpi-simple-soundcard)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-simple-soundcard/description
  This package contains support for Raspbery Pi simple soundcards
endef

$(eval $(call KernelPackage,sound-soc-rpi-simple-soundcard))


define KernelPackage/sound-soc-rpi-wm8804-soundcard
  TITLE:=Support for Raspberry Pi generic WM8804 soundcards
  KCONFIG:= \
    CONFIG_SND_RPI_WM8804_SOUNDCARD
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-wm8804-soundcard.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-rpi-wm8804-soundcard)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-wm8804-soundcard/description
  This package contains support for Raspbery Pi simple soundcards
endef

$(eval $(call KernelPackage,sound-soc-rpi-wm8804-soundcard))


define KernelPackage/sound-soc-adau1977-adc
  TITLE:=Support for ADAU1977 ADC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_ADAU1977_ADC \
    CONFIG_SND_SOC_ADAU1977 \
    CONFIG_SND_SOC_ADAU1977_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-adau1977.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-adau1977-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-adau1977 snd-soc-adau1977-i2c)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-boss-dac/description
  This package contains support for Allo Boss DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-boss-dac))


define KernelPackage/sound-soc-allo-boss2-dac
  TITLE:=Support for Allo Boss2 DAC
  KCONFIG:= \
    CONFIG_SND_AUDIO_GRAPH_CARD \
    CONFIG_SND_BCM2708_SOC_ALLO_BOSS2_DAC
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-allo-boss2-dac.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-allo-boss2-dac)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-boss2-dac/description
  This package contains support for Allo Boss2 DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-boss2-dac))


define KernelPackage/sound-soc-allo-digione
  TITLE:=Support for Allo Piano DigiOne
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_ALLO_DIGIONE \
    CONFIG_SND_SOC_WM8804 \
    CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804-i2c snd-soc-wm8804 \
    snd-soc-allo-digione)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-wm8804-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-piano-dac-plus/description
  This package contains support for Allo Piano DAC Plus
endef

$(eval $(call KernelPackage,sound-soc-allo-piano-dac-plus))


define KernelPackage/sound-soc-audiosense-pi
  TITLE:=Support for AudioSense Add-On Soundcard
  KCONFIG:= \
    CONFIG_SND_AUDIOSENSE_PI \
    CONFIG_SND_SOC_TLV320AIC32X4 \
    CONFIG_SND_SOC_TLV320AIC32X4_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-audiosense-pi.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-tlv320aic32x4.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-tlv320aic32x4-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-tlv320aic32x4-i2c snd-soc-tlv320aic32x4 \
    snd-soc-audiosense-pi)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-audiosense-pi/description
  This package contains support for AudioSense Add-On Soundcard
endef

$(eval $(call KernelPackage,sound-soc-audiosense-pi))


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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-allo-katana-codec/description
  This package contains support for Allo Katana DAC
endef

$(eval $(call KernelPackage,sound-soc-allo-katana-codec))


define KernelPackage/sound-soc-audioinjector-isolated-soundcard
  TITLE:=Support for AudioInjector Isolated soundcard
  KCONFIG:= \
    CONFIG_SND_AUDIOINJECTOR_ISOLATED_SOUNDCARD \
    CONFIG_SND_SOC_CS4271 \
    CONFIG_SND_SOC_CS4271_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-audioinjector-isolated-soundcard.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8731.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-cs4271.o \
    snd-soc-cs4271-i2c \
    snd-soc-audioinjector-isolated-soundcard)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c \
    +kmod-regmap-spi
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-audioinjector-isolated-soundcard/description
  This package contains support for AudioInjector Isolated soundcard
endef

$(eval $(call KernelPackage,sound-soc-audioinjector-isolated-soundcard))


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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c \
    +kmod-regmap-spi
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-audioinjector-pi-soundcard/description
  This package contains support for AudioInjector Pi soundcard
endef

$(eval $(call KernelPackage,sound-soc-audioinjector-pi-soundcard))


define KernelPackage/sound-soc-chipdip-dac
  TITLE:=Support for ChipDip DAC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_CHIPDIP_DAC
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-chipdip-dac.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-chipdip-dac)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-chipdip-dac/description
  This package contains support for ChipDip DAC
endef

$(eval $(call KernelPackage,sound-soc-chipdip-dac))


define KernelPackage/sound-soc-dacberry-soundcard
  TITLE:=Support for DACBERRY400 Soundcard
  KCONFIG:= \
    CONFIG_SND_DACBERRY400 \
    CONFIG_SND_SOC_TLV320AIC3X_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-dacberry400.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-tlv320aic3x.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-tlv320aic3x-i2c snd-soc-dacberry400)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-dacberry-soundcard/description
  This package contains support for DACBERRY400 Soundcard
endef

$(eval $(call KernelPackage,sound-soc-dacberry-soundcard))


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
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c \
    +kmod-regmap-spi
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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-regmap-i2c
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
    kmod-sound-soc-bcm2835-i2s \
    +kmod-regmap-i2c
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
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-googlevoicehat-codec.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-googlevoicehat-codec)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard
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
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm5102a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm5102a)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard \
    +kmod-i2c-bcm2835
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dac/description
  This package contains support for HifiBerry DAC
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dac))


define KernelPackage/sound-soc-hifiberry-dacplus
  TITLE:=Support for HifiBerry DAC+ / DAC+ Pro / Amp2
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUS \
    CONFIG_SND_SOC_PCM512x \
    CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
    $(LINUX_DIR)/drivers/clk/clk-hifiberry-dacpro.ko \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplus.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,clk-hifiberry-dacpro snd-soc-pcm512x \
    snd-soc-pcm512x-i2c snd-soc-hifiberry-dacplus)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplus/description
  This package contains support for HifiBerry DAC+ / DAC+ Pro / Amp2
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplus))


define KernelPackage/sound-soc-hifiberry-dacplusadc
  TITLE:=Support for HifiBerry DAC+ADC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUSADC \
    CONFIG_SND_SOC_PCM512x \
    CONFIG_SND_SOC_DMIC
  FILES:= \
    $(LINUX_DIR)/drivers/clk/clk-hifiberry-dacpro.ko \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplusadc.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-dmic.ko
  AUTOLOAD:=$(call AutoLoad,68,clk-hifiberry-dacpro snd-soc-pcm512x \
    snd-soc-dmic snd-soc-hifiberry-dacplusadc)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplusadc/description
  This package contains support for HifiBerry DAC+ADC
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplusadc))


define KernelPackage/sound-soc-hifiberry-dacplusdsp
  TITLE:=Support for HifiBerry DAC+DSP
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUSDSP
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplusdsp.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-hifiberry-dacplusdsp)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplusdsp/description
  This package contains support for HifiBerry DAC+DSP
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplusdsp))


define KernelPackage/sound-soc-hifiberry-dacplushd
  TITLE:=Support for HifiBerry DAC+HD
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUSHD \
    CONFIG_SND_SOC_PCM179X \
    CONFIG_SND_SOC_PCM179X_I2C
  FILES:= \
    $(LINUX_DIR)/drivers/clk/clk-hifiberry-dachd.ko \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplushd.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm179x-codec.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm179x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,clk-hifiberry-dachd snd-soc-pcm179x-codec \
    snd-soc-pcm179x-i2c snd-soc-hifiberry-dacplushd)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplushd/description
  This package contains support for HifiBerry DAC+HD
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplushd))


define KernelPackage/sound-soc-hifiberry-dacplusadc-pro
  TITLE:=Support for HifiBerry DAC+ADC PRO
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DACPLUSADCPRO \
    CONFIG_SND_SOC_PCM186X \
    CONFIG_SND_SOC_PCM186X_I2C \
    CONFIG_SND_SOC_PCM512x \
    CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-hifiberry-dacplusadcpro.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm186x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm186x-i2c.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm186x snd-soc-pcm186x-i2c \
    snd-soc-pcm512x snd-soc-pcm512x-i2c snd-soc-hifiberry-dacplusadcpro)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-dacplusadc-pro/description
  This package contains support for HifiBerry DAC+ADC PRO
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-dacplusadc-pro))


define KernelPackage/sound-soc-hifiberry-digi
  TITLE:=Support for HifiBerry Digi / Digi+ / Digi+ Pro
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_HIFIBERRY_DIGI \
    CONFIG_SND_SOC_WM8804 \
    CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804 snd-soc-wm8804-i2c)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-wm8804-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-tas5713.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-tas5713)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-hifiberry-amp/description
  This package contains support for HifiBerry Amp
endef

$(eval $(call KernelPackage,sound-soc-hifiberry-amp))


define KernelPackage/sound-soc-iqaudio-codec
  TITLE:=Support for IQaudIO-CODEC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_IQAUDIO_CODEC \
    CONFIG_SND_SOC_DA7213
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-iqaudio-codec.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-da7213.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-da7213 snd-soc-iqaudio-codec)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-iqaudio-codec/description
  This package contains support for IQaudIO-CODEC
endef

$(eval $(call KernelPackage,sound-soc-iqaudio-codec))


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
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804 snd-soc-wm8804-i2c)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-wm8804-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-iqaudio-digi/description
  This package contains support for IQaudIO-DIGI
endef

$(eval $(call KernelPackage,sound-soc-iqaudio-digi))


define KernelPackage/sound-soc-i-sabe-q2m
  TITLE:=Support for Audiophonics I-Sabre Q2M DAC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_I_SABRE_Q2M \
    CONFIG_SND_SOC_I_SABRE_CODEC
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-i-sabre-q2m.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-i-sabre-codec.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-i-sabre-codec snd-soc-i-sabre-q2m)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-i-sabe-q2m/description
  This package contains support for Audiophonics I-SABRE Q2M DAC
endef

$(eval $(call KernelPackage,sound-soc-i-sabe-q2m))


define KernelPackage/sound-soc-justboom-both
  TITLE:=Support for JustBoom DAC and Digi
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_JUSTBOOM_BOTH \
    CONFIG_SND_SOC_PCM512x \
    CONFIG_SND_SOC_PCM512x_I2C \
    CONFIG_SND_SOC_WM8804 \
    CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-justboom-both.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
    snd-soc-wm8804-i2c snd-soc-wm8804 \
    snd-soc-justboom-both)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-wm8804-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-justboom-both/description
  This package contains support for JustBoom DAC and Digi
endef

$(eval $(call KernelPackage,sound-soc-justboom-both))


define KernelPackage/sound-soc-justboom-dac
  TITLE:=Support for JustBoom DAC
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_JUSTBOOM_DAC \
    CONFIG_SND_SOC_PCM512x \
    CONFIG_SND_SOC_PCM512x_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-justboom-dac.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm512x-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm512x-i2c snd-soc-pcm512x \
    snd-soc-justboom-dac)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    CONFIG_SND_SOC_WM8804 \
    CONFIG_SND_SOC_WM8804_I2C
  FILES:= \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804-i2c.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-wm8804-i2c snd-soc-wm8804)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-wm8804-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-justboom-digi/description
  This package contains support for JustBoom Digi
endef

$(eval $(call KernelPackage,sound-soc-justboom-digi))


define KernelPackage/sound-soc-pifi-40-amp
  TITLE:=Support for PiFi-40 amp
  KCONFIG:= \
    CONFIG_SND_BCM2708_SOC_PIFI_40 \
    CONFIG_SND_PIFI_40 \
    CONFIG_SND_SOC_TAS571X
  FILES:= \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-pifi-40.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-tas571x.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-tas571x)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-pifi-40-amp/description
  This package contains support for PiFi-40 amp
endef

$(eval $(call KernelPackage,sound-soc-pifi-40-amp))


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


define KernelPackage/sound-soc-rpi-cirrus
  TITLE:=Support for Cirrus Logic Audio Card
  KCONFIG:= \
    CONFIG_GPIO_ARIZONA \
    CONFIG_INPUT_ARIZONA_HAPTICS=n \
    CONFIG_MFD_ARIZONA=y \
    CONFIG_MFD_ARIZONA_I2C \
    CONFIG_MFD_CS47L24=n \
    CONFIG_MFD_WM5102=y \
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
    $(LINUX_DIR)/drivers/firmware/cirrus/cs_dsp.ko \
    $(LINUX_DIR)/drivers/mfd/arizona.ko \
    $(LINUX_DIR)/drivers/mfd/arizona-i2c.ko \
    $(LINUX_DIR)/sound/soc/bcm/snd-soc-rpi-cirrus.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-arizona.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm-adsp.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm5102.ko \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8804.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-rpi-cirrus)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
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
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-pcm1794a.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-pcm1794a)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard \
    +kmod-i2c-bcm2835
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-dac/description
  This package contains support for RPi-DAC
endef

$(eval $(call KernelPackage,sound-soc-rpi-dac))


define KernelPackage/sound-soc-merus-amp
  TITLE:=Support for Infineon Merus Amp
  KCONFIG:= \
    CONFIG_SND_SOC_MA120X0P
  FILES:= \
    $(LINUX_DIR)/sound/soc/codecs/snd-soc-ma120x0p.ko
  AUTOLOAD:=$(call AutoLoad,68,snd-soc-ma120x0p)
  DEPENDS:= \
    kmod-sound-soc-bcm2835-i2s \
    +kmod-sound-soc-rpi-simple-soundcard \
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-merus-amp/description
  This package contains support for Infineon Merus Amp
endef

$(eval $(call KernelPackage,sound-soc-merus-amp))


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
    +kmod-i2c-bcm2835 \
    +kmod-regmap-i2c \
    +kmod-regmap-spi
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-rpi-proto/description
  This package contains support for RPi-PROTO
endef

$(eval $(call KernelPackage,sound-soc-rpi-proto))
