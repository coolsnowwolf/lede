# SPDX-License-Identifier: GPL-2.0-or-later

DEVICE_VARS += CFE_BOARD_ID CFE_EXTRAS
DEVICE_VARS += FLASH_MB IMAGE_OFFSET

define Device/bcm63xx-cfe
  FILESYSTEMS := squashfs jffs2-64k jffs2-128k
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma elf
  KERNEL_INITRAMFS_SUFFIX := .elf
  IMAGES := cfe.bin sysupgrade.bin
  IMAGE/cfe.bin := \
    cfe-bin $$$$(if $$$$(FLASH_MB),--pad $$$$(shell expr $$$$(FLASH_MB) / 2))
  IMAGE/sysupgrade.bin := cfe-bin | append-metadata
  BLOCKSIZE := 0x10000
  IMAGE_OFFSET :=
  FLASH_MB :=
  CFE_BOARD_ID :=
  CFE_EXTRAS = --block-size $$(BLOCKSIZE) \
    --image-offset $$(if $$(IMAGE_OFFSET),$$(IMAGE_OFFSET),$$(BLOCKSIZE))
endef

# Legacy CFEs with specific LZMA parameters and no length
define Device/bcm63xx-cfe-legacy
  $(Device/bcm63xx-cfe)
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma-cfe
endef

define Device/comtrend_ar-5315u
  $(Device/bcm63xx-cfe)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5315u
  CHIP_ID := 6318
  CFE_BOARD_ID := 96318A-1441N1
  FLASH_MB := 16
  DEVICE_PACKAGES += $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += comtrend_ar-5315u

define Device/comtrend_ar-5387un
  $(Device/bcm63xx-cfe)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5387un
  CHIP_ID := 6328
  CFE_BOARD_ID := 96328A-1441N1
  FLASH_MB := 16
  DEVICE_PACKAGES += $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += comtrend_ar-5387un

define Device/comtrend_vr-3025u
  $(Device/bcm63xx-cfe)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3025u
  CHIP_ID := 6368
  CFE_BOARD_ID := 96368M-1541N
  BLOCKSIZE := 0x20000
  FLASH_MB := 32
  DEVICE_PACKAGES += $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += comtrend_vr-3025u

define Device/huawei_hg556a-b
  $(Device/bcm63xx-cfe-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := B
  CHIP_ID := 6358
  CFE_BOARD_ID := HW556
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  BLOCKSIZE := 0x20000
  DEVICE_PACKAGES += $(USB2_PACKAGES) $(ATH9K_PACKAGES)
endef
TARGET_DEVICES += huawei_hg556a-b
