AT91_SD_BOOT_PARTSIZE:=64
FAT32_BLOCK_SIZE:=1024
FAT32_BLOCKS:=$(shell echo \
  $$(($(AT91_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/at91-sdcard
  rm -f $@.boot
  mkfs.fat -C $@.boot $(FAT32_BLOCKS)

  mcopy -i $@.boot $(KDIR)/zImage ::zImage

  $(foreach dts,$(DEVICE_DTS), \
     mcopy -i $@.boot $(DTS_DIR)/$(dts).dtb \
        ::$(dts).dtb; \
     mcopy -i $@.boot \
        $(BIN_DIR)/u-boot-$(dts:at91-%=%)_mmc/u-boot.bin \
            ::u-boot.bin; \
     $(CP) $(BIN_DIR)/at91bootstrap-$(dts:at91-%=%)sd_uboot*/*.bin \
         $(BIN_DIR)/BOOT.bin; \
     mcopy -i $@.boot $(BIN_DIR)/BOOT.bin ::BOOT.bin;)

  ./gen_at91_sdcard_img.sh \
      $(dir $@)$(IMG_PREFIX)-$(DEVICE_NAME)-sdcard.img \
      $@.boot \
      $(KDIR)/root.ext4 \
      $(AT91_SD_BOOT_PARTSIZE) \
      $(CONFIG_TARGET_ROOTFS_PARTSIZE)

  gzip -nc9 $(dir $@)$(IMG_PREFIX)-$(DEVICE_NAME)-sdcard.img \
         > $(dir $@)$(IMG_PREFIX)-$(DEVICE_NAME)-sdcard.img.gz

  $(CP) $(dir $@)$(IMG_PREFIX)-$(DEVICE_NAME)-sdcard.img.gz \
        $(BIN_DIR)/

  rm -f $(BIN_DIR)/BOOT.bin
  rm -f $@.boot
endef

define Device/evaluation-sdimage
  IMAGES += sdcard.img.gz
  IMAGE/sdcard.img.gz := at91-sdcard
endef

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

define Device/at91-sama5d2_xplained
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D2 Xplained
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d2_xplained

define Device/at91-sama5d4_xplained
  $(Device/evaluation-dtb)
  DEVICE_TITLE := Microchip(Atmel AT91) SAMA5D4 Xplained
  KERNEL_SIZE := 6144k
  BLOCKSIZE := 256k
  PAGESIZE := 4096
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 248KiB -c 2082 -x lzo
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += at91-sama5d4_xplained

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
