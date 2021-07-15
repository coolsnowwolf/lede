include ./common-buffalo.mk
include ./common-senao.mk

define Device/buffalo_whr-g301n
  $(Device/buffalo_common)
  SOC := ar7240
  DEVICE_MODEL := WHR-G301N
  BUFFALO_PRODUCT := WHR-G301N
  IMAGE_SIZE := 3712k
  SUPPORTED_DEVICES += whr-g301n
  DEFAULT := n
endef
TARGET_DEVICES += buffalo_whr-g301n

define Device/dlink_dir-615-e4
  SOC := ar7240
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-615
  DEVICE_VARIANT := E4
  IMAGE_SIZE := 3776k
  FACTORY_IMAGE_SIZE := 3456k
  IMAGES += factory.bin
  IMAGE/default := append-kernel | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | check-size | append-metadata
  IMAGE/factory.bin := $$(IMAGE/default) | \
	check-size $$$$(FACTORY_IMAGE_SIZE) | pad-to $$$$(FACTORY_IMAGE_SIZE) | \
	append-string "AP99-AR7240-RT-091105-05"
  SUPPORTED_DEVICES += dir-615-e4
  DEFAULT := n
endef
TARGET_DEVICES += dlink_dir-615-e4

define Device/engenius_eap350-v1
  $(Device/senao_loader_okli)
  BLOCKSIZE := 4k
  SOC := ar7242
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := EAP350
  DEVICE_VARIANT := v1
  IMAGE_SIZE := 4928k
  LOADER_FLASH_OFFS := 0x1a0000
  SENAO_IMGNAME := senao-eap350
endef
TARGET_DEVICES += engenius_eap350-v1

define Device/engenius_ecb350-v1
  $(Device/senao_loader_okli)
  BLOCKSIZE := 4k
  SOC := ar7242
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ECB350
  DEVICE_VARIANT := v1
  IMAGE_SIZE := 4928k
  LOADER_FLASH_OFFS := 0x1a0000
  SENAO_IMGNAME := senao-ecb350
endef
TARGET_DEVICES += engenius_ecb350-v1

define Device/engenius_enh202-v1
  $(Device/senao_loader_okli)
  SOC := ar7240
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ENH202
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := rssileds
  IMAGE_SIZE := 4928k
  LOADER_FLASH_OFFS := 0x1a0000
  SENAO_IMGNAME := senao-enh202
endef
TARGET_DEVICES += engenius_enh202-v1

define Device/pqi_air-pen
  SOC := ar9330
  DEVICE_VENDOR := PQI
  DEVICE_MODEL := Air-Pen
  DEVICE_PACKAGES := kmod-usb2
  IMAGE_SIZE := 7680k
  SUPPORTED_DEVICES += pqi-air-pen
endef
TARGET_DEVICES += pqi_air-pen
