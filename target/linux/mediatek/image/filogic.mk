DTS_DIR := $(DTS_DIR)/mediatek

KERNEL_LOADADDR := 0x44000000

define Image/Prepare
	# For UBI we want only one extra block
	rm -f $(KDIR)/ubi_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/ubi_mark
endef

define Device/mediatek_mt7986a-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 rfba AP
  DEVICE_DTS := mt7986a-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/
  KERNEL_LOADADDR := 0x48000000
  DEVICE_DTS_OVERLAY := mt7986a-rfb-spim-nand mt7986a-rfb-spim-nor
  SUPPORTED_DEVICES := mediatek,mt7986a-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb with-initrd
  DTC_FLAGS += -@ --space 32768
endef
TARGET_DEVICES += mediatek_mt7986a-rfb

define Device/mediatek_mt7986b-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 rfbb AP
  DEVICE_DTS := mt7986b-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/
  KERNEL_LOADADDR := 0x48000000
  SUPPORTED_DEVICES := mediatek,mt7986b-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mediatek_mt7986b-rfb
