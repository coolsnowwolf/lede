#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

SOUND_MENU:=Sound Support

# allow targets to override the soundcore stuff
SOUNDCORE_LOAD ?= \
	soundcore \
	snd \
	snd-hwdep \
	snd-seq-device \
	snd-rawmidi \
	snd-timer \
	snd-pcm \
	snd-mixer-oss \
	snd-pcm-oss \
	snd-compress

SOUNDCORE_FILES ?= \
	$(LINUX_DIR)/sound/soundcore.ko \
	$(LINUX_DIR)/sound/core/snd.ko \
	$(LINUX_DIR)/sound/core/snd-hwdep.ko \
	$(LINUX_DIR)/sound/core/snd-seq-device.ko \
	$(LINUX_DIR)/sound/core/snd-rawmidi.ko \
	$(LINUX_DIR)/sound/core/snd-timer.ko \
	$(LINUX_DIR)/sound/core/snd-pcm.ko \
	$(LINUX_DIR)/sound/core/oss/snd-mixer-oss.ko \
	$(LINUX_DIR)/sound/core/oss/snd-pcm-oss.ko \
	$(LINUX_DIR)/sound/core/snd-compress.ko

SOUNDCORE_LOAD += \
	$(if $(CONFIG_SND_DMAENGINE_PCM),snd-pcm-dmaengine)

SOUNDCORE_FILES += \
	$(if $(CONFIG_SND_DMAENGINE_PCM),$(LINUX_DIR)/sound/core/snd-pcm-dmaengine.ko)

define KernelPackage/sound-core
  SUBMENU:=$(SOUND_MENU)
  TITLE:=Sound support
  DEPENDS:=@AUDIO_SUPPORT +kmod-input-core
  KCONFIG:= \
	CONFIG_SOUND \
	CONFIG_SND \
	CONFIG_SND_HWDEP \
	CONFIG_SND_RAWMIDI \
	CONFIG_SND_TIMER \
	CONFIG_SND_PCM \
	CONFIG_SND_PCM_TIMER=y \
	CONFIG_SND_SEQUENCER \
	CONFIG_SND_VIRMIDI \
	CONFIG_SND_SEQ_DUMMY \
	CONFIG_SND_SEQUENCER_OSS=y \
	CONFIG_HOSTAUDIO \
	CONFIG_SND_PCM_OSS \
	CONFIG_SND_MIXER_OSS \
	CONFIG_SOUND_OSS_CORE_PRECLAIM=y \
	CONFIG_SND_COMPRESS_OFFLOAD
  FILES:=$(SOUNDCORE_FILES)
  AUTOLOAD:=$(call AutoLoad,30,$(SOUNDCORE_LOAD))
endef

define KernelPackage/sound-core/uml
  FILES:= \
	$(LINUX_DIR)/sound/soundcore.ko \
	$(LINUX_DIR)/arch/um/drivers/hostaudio.ko
  AUTOLOAD+=$(call AutoLoad,30,soundcore hostaudio)
endef

define KernelPackage/sound-core/description
 Kernel modules for sound support
endef

$(eval $(call KernelPackage,sound-core))


define AddDepends/sound
  SUBMENU:=$(SOUND_MENU)
  DEPENDS+=kmod-sound-core $(1) @!TARGET_uml
endef


define KernelPackage/ac97
  TITLE:=ac97 controller
  KCONFIG:=CONFIG_SND_AC97_CODEC
  FILES:= \
	$(LINUX_DIR)/sound/ac97_bus.ko \
	$(LINUX_DIR)/sound/pci/ac97/snd-ac97-codec.ko
  AUTOLOAD:=$(call AutoLoad,35,ac97_bus snd-ac97-codec)
  $(call AddDepends/sound)
endef

define KernelPackage/ac97/description
 The ac97 controller
endef

$(eval $(call KernelPackage,ac97))


define KernelPackage/sound-mpu401
  TITLE:=MPU-401 uart driver
  KCONFIG:=CONFIG_SND_MPU401_UART
  FILES:= \
	$(LINUX_DIR)/sound/drivers/mpu401/snd-mpu401-uart.ko
  AUTOLOAD:=$(call AutoLoad,35,snd-mpu401-uart)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-mpu401/description
 support for MIDI ports compatible with the Roland MPU-401
 interface in UART mode.
endef

$(eval $(call KernelPackage,sound-mpu401))


define KernelPackage/sound-seq
  TITLE:=Sequencer support
  FILES:= \
	$(LINUX_DIR)/sound/core/seq/snd-seq.ko \
	$(LINUX_DIR)/sound/core/seq/snd-seq-midi-event.ko \
	$(LINUX_DIR)/sound/core/seq/snd-seq-midi.ko
  AUTOLOAD:=$(call AutoLoad,35,snd-seq snd-seq-midi-event snd-seq-midi)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-seq/description
 Kernel modules for sequencer support
endef

$(eval $(call KernelPackage,sound-seq))


define KernelPackage/sound-ens1371
  TITLE:=(Creative) Ensoniq AudioPCI 1371
  KCONFIG:=CONFIG_SND_ENS1371
  DEPENDS:=@PCI_SUPPORT +kmod-ac97
  FILES:=$(LINUX_DIR)/sound/pci/snd-ens1371.ko
  AUTOLOAD:=$(call AutoLoad,36,snd-ens1371)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-ens1371/description
 support for (Creative) Ensoniq AudioPCI 1371 chips
endef

$(eval $(call KernelPackage,sound-ens1371))


define KernelPackage/sound-i8x0
  TITLE:=Intel/SiS/nVidia/AMD/ALi AC97 Controller
  DEPENDS:=+kmod-ac97
  KCONFIG:=CONFIG_SND_INTEL8X0
  FILES:=$(LINUX_DIR)/sound/pci/snd-intel8x0.ko
  AUTOLOAD:=$(call AutoLoad,36,snd-intel8x0)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-i8x0/description
 support for the integrated AC97 sound device on motherboards
 with Intel/SiS/nVidia/AMD chipsets, or ALi chipsets using
 the M5455 Audio Controller.
endef

$(eval $(call KernelPackage,sound-i8x0))


define KernelPackage/sound-via82xx
  TITLE:=VIA 82xx AC97 Controller
  DEPENDS:=+kmod-ac97 +kmod-sound-mpu401
  KCONFIG:=CONFIG_SND_VIA82XX
  FILES:=$(LINUX_DIR)/sound/pci/snd-via82xx.ko
  AUTOLOAD:=$(call AutoLoad,36,snd-via82xx)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-via82xx/description
 support for the integrated AC97 sound device on motherboards
 with VIA chipsets.
endef

$(eval $(call KernelPackage,sound-via82xx))


define KernelPackage/sound-soc-core
  TITLE:=SoC sound support
  DEPENDS:=+kmod-regmap-core +kmod-ac97
  KCONFIG:= \
	CONFIG_SND_SOC \
	CONFIG_SND_SOC_ADI=n \
	CONFIG_SND_SOC_GENERIC_DMAENGINE_PCM=y \
	CONFIG_SND_SOC_ALL_CODECS=n
  FILES:=$(LINUX_DIR)/sound/soc/snd-soc-core.ko
  AUTOLOAD:=$(call AutoLoad,55,snd-soc-core)
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-core))


define KernelPackage/sound-soc-ac97
  TITLE:=AC97 Codec support
  KCONFIG:=CONFIG_SND_SOC_AC97_CODEC
  FILES:=$(LINUX_DIR)/sound/soc/codecs/snd-soc-ac97.ko
  AUTOLOAD:=$(call AutoLoad,57,snd-soc-ac97)
  DEPENDS:=+kmod-ac97 +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-ac97))


define KernelPackage/sound-soc-imx
  TITLE:=IMX SoC support
  KCONFIG:=\
	CONFIG_SND_IMX_SOC \
	CONFIG_SND_SOC_IMX_AUDMUX \
	CONFIG_SND_SOC_FSL_SSI \
	CONFIG_SND_SOC_IMX_PCM_DMA
  FILES:= \
	$(LINUX_DIR)/sound/soc/fsl/snd-soc-imx-audmux.ko \
	$(LINUX_DIR)/sound/soc/fsl/snd-soc-fsl-ssi.ko \
	$(LINUX_DIR)/sound/soc/fsl/imx-pcm-dma.ko
  AUTOLOAD:=$(call AutoLoad,56,snd-soc-imx-audmux snd-soc-fsl-ssi snd-soc-imx-pcm)
  DEPENDS:=@TARGET_imx +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-imx/description
 Support for i.MX Platform sound (ssi/audmux/pcm)
endef

$(eval $(call KernelPackage,sound-soc-imx))


define KernelPackage/sound-soc-mt7986
  TITLE:=MediaTek MT7986 Audio support
  KCONFIG:=CONFIG_SND_SOC_MT7986 CONFIG_SND_SOC_MT7986_WM8960
  FILES:= \
	$(LINUX_DIR)/sound/soc/mediatek/common/snd-soc-mtk-common.ko \
	$(LINUX_DIR)/sound/soc/mediatek/mt7986/snd-soc-mt7986-afe.ko
  AUTOLOAD:=$(call AutoLoad,56,snd-soc-mtk-common snd-soc-mt7986-afe)
  DEPENDS:=@TARGET_mediatek_filogic +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-mt7986/description
 Support for audio on systems using the MediaTek MT7986 SoC.
endef

$(eval $(call KernelPackage,sound-soc-mt7986))


define KernelPackage/sound-soc-mt7986-wm8960
  TITLE:=MediaTek MT7986 Audio support
  KCONFIG:=CONFIG_SND_SOC_MT7986_WM8960
  FILES:=$(LINUX_DIR)/sound/soc/mediatek/mt7986/mt7986-wm8960.ko
  AUTOLOAD:=$(call AutoLoad,57,mt7986-wm8960)
  DEPENDS:=@TARGET_mediatek_filogic +kmod-sound-soc-wm8960 +kmod-sound-soc-mt7986
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-mt7986-wm8960/description
 Support for use of the Wolfson Audio WM8960 codec with the MediaTek MT7986 SoC.
endef

$(eval $(call KernelPackage,sound-soc-mt7986-wm8960))


define KernelPackage/sound-soc-imx-sgtl5000
  TITLE:=IMX SoC support for SGTL5000
  KCONFIG:=CONFIG_SND_SOC_IMX_SGTL5000
  FILES:=\
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-sgtl5000.ko \
	$(LINUX_DIR)/sound/soc/fsl/snd-soc-imx-sgtl5000.ko
  AUTOLOAD:=$(call AutoLoad,57,snd-soc-sgtl5000 snd-soc-imx-sgtl5000)
  DEPENDS:=@TARGET_imx +kmod-sound-soc-imx +kmod-regmap-i2c
  $(call AddDepends/sound)
endef

define KernelPackage/sound-soc-imx-sgtl5000/description
 Support for i.MX Platform sound SGTL5000 codec
endef

$(eval $(call KernelPackage,sound-soc-imx-sgtl5000))


define KernelPackage/sound-soc-wm8960
  TITLE:=SoC WM8960 codec support
  KCONFIG:=CONFIG_SND_SOC_WM8960
  FILES:=$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8960.ko
  DEPENDS:=+kmod-sound-soc-core +kmod-i2c-core +kmod-regmap-i2c
  AUTOLOAD:=$(call AutoProbe,snd-soc-wm8960)
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-wm8960))


define KernelPackage/sound-soc-spdif
  TITLE:=SoC S/PDIF codec support
  KCONFIG:=CONFIG_SND_SOC_SPDIF
  FILES:= \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-spdif-tx.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-spdif-rx.ko
  DEPENDS:=+kmod-sound-soc-core
  AUTOLOAD:=$(call AutoProbe,snd-soc-spdif-tx snd-soc-spdif-rx)
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-spdif))


define KernelPackage/pcspkr
  DEPENDS:=@TARGET_x86 +kmod-input-core
  TITLE:=PC speaker support
  KCONFIG:= \
	CONFIG_SND_PCSP
  FILES:= \
	$(LINUX_DIR)/sound/drivers/pcsp/snd-pcsp.ko
  AUTOLOAD:=$(call AutoLoad,50,snd-pcsp)
  $(call AddDepends/sound)
endef

define KernelPackage/pcspkr/description
 This enables sounds (tones) through the pc speaker
endef

$(eval $(call KernelPackage,pcspkr))

define KernelPackage/sound-dummy
  $(call AddDepends/sound)
  TITLE:=Null sound output driver (sink)
  KCONFIG:= \
	CONFIG_SND_DUMMY
  FILES:= \
	$(LINUX_DIR)/sound/drivers/snd-dummy.ko
  AUTOLOAD:=$(call AutoLoad,32,snd-dummy)
endef

define KernelPackage/sound-dummy/description
 Dummy sound device for Alsa when no hardware present
endef

$(eval $(call KernelPackage,sound-dummy))

define KernelPackage/sound-hda-core
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Sound Core Support
  DEPENDS:=+kmod-ledtrig-audio
  KCONFIG:= \
	CONFIG_SND_HDA_CORE \
	CONFIG_SND_HDA_HWDEP=y \
	CONFIG_SND_HDA_RECONFIG=n \
	CONFIG_SND_HDA_SCODEC_COMPONENT=y \
	CONFIG_SND_HDA_INPUT_BEEP=n \
	CONFIG_SND_HDA_PATCH_LOADER=n \
	CONFIG_SND_HDA_GENERIC
  FILES:= \
	$(LINUX_DIR)/sound/hda/snd-hda-core.ko \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec.ko \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-generic.ko \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-scodec-component.ko@ge6.12
  AUTOLOAD:=$(call AutoProbe,snd-hda-core snd-hda-codec snd-hda-codec-generic)
  $(call AddDepends/sound,+kmod-regmap-core)
endef

define KernelPackage/sound-hda-core/description
 Kernel modules for HD Audio sound support
endef

$(eval $(call KernelPackage,sound-hda-core))

define KernelPackage/sound-hda-codec-realtek
  SUBMENU:=$(SOUND_MENU)
  TITLE:= HD Audio Realtek Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_REALTEK
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-realtek.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-realtek)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-realtek/description
 Kernel modules for Intel HDA Realtek codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-realtek))

define KernelPackage/sound-hda-codec-cmedia
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio C-Media Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_CMEDIA
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-cmedia.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-cmedia)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-cmedia/description
 Kernel modules for HD Audio C-Media codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-cmedia))

define KernelPackage/sound-hda-codec-analog
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Analog Devices Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_ANALOG
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-analog.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-analog)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-analog/description
 Kernel modules for HD Audio Analog Devices codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-analog))

define KernelPackage/sound-hda-codec-idt
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Sigmatel IDT Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_SIGMATEL
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-idt.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-idt)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-idt/description
 Kernel modules for HD Audio Sigmatel IDT codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-idt))

define KernelPackage/sound-hda-codec-si3054
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Silicon Labs 3054 Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_SI3054
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-si3054.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-si3054)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-si3054/description
 Kernel modules for HD Audio Silicon Labs 3054 codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-si3054))

define KernelPackage/sound-hda-codec-cirrus
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Cirrus Logic Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_CIRRUS
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-cirrus.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-cirrus)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-cirrus/description
 Kernel modules for HD Audio Cirrus Logic codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-cirrus))

define KernelPackage/sound-hda-codec-ca0110
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Creative CA0110 Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_CA0110
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-ca0110.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-ca0110)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-ca0110/description
 Kernel modules for HD Audio Creative CA0110 codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-ca0110))

define KernelPackage/sound-hda-codec-ca0132
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Creative CA0132 Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_CA0132 \
	CONFIG_SND_HDA_CODEC_CA0132_DSP=n
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-ca0132.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-ca0132)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-ca0132/description
 Kernel modules for HD Audio Creative CA0132 codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-ca0132))

define KernelPackage/sound-hda-codec-conexant
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Conexant Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_CONEXANT
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-conexant.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-conexant)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-conexant/description
 Kernel modules for HD Audio Conexant codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-conexant))

define KernelPackage/sound-hda-codec-via
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Via Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_VIA
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-via.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-via)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-via/description
 Kernel modules for HD Audio VIA codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-via))

define KernelPackage/sound-hda-codec-hdmi
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio HDMI/DisplayPort Codec
  KCONFIG:= \
	CONFIG_SND_HDA_CODEC_HDMI
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-codec-hdmi.ko
  AUTOLOAD:=$(call AutoProbe,snd-hda-codec-hdmi)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-codec-hdmi/description
 Kernel modules for HD Audio HDMI codec support
endef

$(eval $(call KernelPackage,sound-hda-codec-hdmi))

define KernelPackage/sound-hda-intel
  SUBMENU:=$(SOUND_MENU)
  TITLE:=HD Audio Intel Driver
  DEPENDS:=@TARGET_x86
  KCONFIG:= \
	CONFIG_SOUND_PCI \
	CONFIG_SND_HDA_INTEL
  FILES:= \
	$(LINUX_DIR)/sound/pci/hda/snd-hda-intel.ko \
	$(LINUX_DIR)/sound/hda/snd-intel-nhlt.ko@lt5.5 \
	$(LINUX_DIR)/sound/hda/snd-intel-dspcfg.ko@ge5.5
  AUTOLOAD:=$(call AutoProbe,snd-hda-intel)
  $(call AddDepends/sound,kmod-sound-hda-core)
endef

define KernelPackage/sound-hda-intel/description
 Kernel modules for HD Audio Intel driver support
endef

$(eval $(call KernelPackage,sound-hda-intel))
