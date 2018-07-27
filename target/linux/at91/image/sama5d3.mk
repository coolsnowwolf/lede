
define Device/default-nand
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 2048
endef

define Device/at91-sama5d3_xplained
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D3 Xplained
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d3_xplained

ifeq ($(strip $(CONFIG_EXTERNAL_KERNEL_TREE)),"")
 ifeq ($(strip $(CONFIG_KERNEL_GIT_CLONE_URI)),"")
  define Device/wb50n
    $(Device/evaluation-fit)
    DEVICE_TITLE := Laird WB50N
    DEVICE_PACKAGES := \
	  kmod-mmc-at91 kmod-ath6kl-sdio ath6k-firmware \
	  kmod-usb-storage kmod-fs-vfat kmod-fs-msdos \
	  kmod-leds-gpio
    BLOCKSIZE := 128k
    PAGESIZE := 2048
    SUBPAGESIZE := 2048
    MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 955
  endef
  TARGET_DEVICES += wb50n
 endif
endif
