define Device/at91-sama5d3_xplained
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Atmel AT91SAMA5D3XPLAINED
  KERNEL_SIZE := 6144k
  SUBPAGESIZE := 2048
endef
TARGET_DEVICES += at91-sama5d3_xplained

