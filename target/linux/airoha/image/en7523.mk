KERNEL_LOADADDR := 0x80208000

define Target/Description
	Build firmware images for Airoha EN7523 ARM based boards.
endef

define Device/airoha_en7523-evb
  DEVICE_VENDOR := Airoha
  DEVICE_MODEL := EN7523 Evaluation Board
  DEVICE_DTS := en7523-evb
  DEVICE_DTS_DIR := ../dts
endef
TARGET_DEVICES += airoha_en7523-evb