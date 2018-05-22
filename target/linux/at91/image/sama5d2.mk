
define Device/default-nand
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 2048
endef

define Device/at91-sama5d2_xplained
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D2 Xplained
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d2_xplained

define Device/at91-sama5d27_som1_ek
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D27 SOM1 Ek
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d27_som1_ek

define Device/at91-sama5d2_ptc_ek
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D2 PTC Ek
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d2_ptc_ek
