# Flashrom variants

define DefaultProgrammer
  MAKE_FLAGS += CONFIG_DEFAULT_PROGRAMMER=PROGRAMMER_$(1)
endef

ifeq ($(BUILD_VARIANT),full)
  $(eval $(call DefaultProgrammer,LINUX_SPI))
  FLASHROM_BASIC := true
  FLASHROM_FTDI := true
  FLASHROM_PCI := true
  FLASHROM_RAW := $(if $(findstring x86,$(CONFIG_ARCH)),true,false)
  FLASHROM_SERIAL := true
  FLASHROM_USB := true
endif
ifeq ($(BUILD_VARIANT),pci)
  $(eval $(call DefaultProgrammer,INTERNAL))
  FLASHROM_BASIC := true
  FLASHROM_FTDI := false
  FLASHROM_PCI := true
  FLASHROM_RAW := $(if $(findstring x86,$(CONFIG_ARCH)),true,false)
  FLASHROM_SERIAL := false
  FLASHROM_USB := false
endif
ifeq ($(BUILD_VARIANT),spi)
  $(eval $(call DefaultProgrammer,LINUX_SPI))
  FLASHROM_BASIC := true
  FLASHROM_FTDI := false
  FLASHROM_PCI := false
  FLASHROM_RAW := false
  FLASHROM_SERIAL := false
  FLASHROM_USB := false
endif
ifeq ($(BUILD_VARIANT),usb)
  $(eval $(call DefaultProgrammer,SERPROG))
  FLASHROM_BASIC := true
  FLASHROM_FTDI := true
  FLASHROM_PCI := false
  FLASHROM_RAW := false
  FLASHROM_SERIAL := true
  FLASHROM_USB := true
endif

MESON_ARGS += \
	-Dconfig_dummy=$(FLASHROM_BASIC) \
	-Dconfig_linux_mtd=$(FLASHROM_BASIC) \
	-Dconfig_linux_spi=$(FLASHROM_BASIC) \
	-Dconfig_mstarddc_spi=$(FLASHROM_BASIC) \
	\
	-Dconfig_ft2232_spi=$(FLASHROM_FTDI) \
	-Dconfig_usbblaster_spi=$(FLASHROM_FTDI) \
	\
	-Dpciutils=$(FLASHROM_PCI) \
	-Dconfig_atavia=$(FLASHROM_PCI) \
	-Dconfig_drkaiser=$(FLASHROM_PCI) \
	-Dconfig_gfxnvidia=$(FLASHROM_PCI) \
	-Dconfig_internal=$(FLASHROM_PCI) \
	-Dconfig_it8212=$(FLASHROM_PCI) \
	-Dconfig_nicintel=$(FLASHROM_PCI) \
	-Dconfig_nicintel_spi=$(FLASHROM_PCI) \
	-Dconfig_nicintel_eeprom=$(FLASHROM_PCI) \
	-Dconfig_ogp_spi=$(FLASHROM_PCI) \
	-Dconfig_satasii=$(FLASHROM_PCI) \
	\
	-Dconfig_bitbang_spi=$(FLASHROM_RAW) \
	-Dconfig_rayer_spi=$(FLASHROM_RAW) \
	\
	-Dconfig_buspirate_spi=$(FLASHROM_SERIAL) \
	-Dconfig_pony_spi=$(FLASHROM_SERIAL) \
	-Dconfig_serprog=$(FLASHROM_SERIAL) \
	\
	-Dusb=$(FLASHROM_USB) \
	-Dconfig_ch341a_spi=$(FLASHROM_USB) \
	-Dconfig_dediprog=$(FLASHROM_USB) \
	-Dconfig_developerbox_spi=$(FLASHROM_USB) \
	-Dconfig_digilent_spi=$(FLASHROM_USB) \
	-Dconfig_pickit2_spi=$(FLASHROM_USB) \
	-Dconfig_stlinkv3_spi=$(FLASHROM_USB)

# PCI
ifeq ($(findstring i386,$(CONFIG_ARCH))$(findstring x86,$(CONFIG_ARCH)),)
MESON_ARGS += \
	-Dconfig_atahpt=false \
	-Dconfig_atapromise=false \
	-Dconfig_internal_dmi=true \
	-Dconfig_nic3com=false \
	-Dconfig_nicnatsemi=false \
	-Dconfig_nicrealtek=false \
	-Dconfig_satamv=false
else
MESON_ARGS += \
	-Dconfig_atahpt=$(FLASHROM_PCI) \
	-Dconfig_atapromise=$(FLASHROM_PCI) \
	-Dconfig_internal_dmi=$(if $(FLASHROM_PCI),false,true)
	-Dconfig_nic3com=$(FLASHROM_PCI) \
	-Dconfig_nicnatsemi=$(FLASHROM_PCI) \
	-Dconfig_nicrealtek=$(FLASHROM_PCI) \
	-Dconfig_satamv=$(FLASHROM_PCI)
endif
