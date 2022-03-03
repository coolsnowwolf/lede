
define Device/default-nand
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 2048
endef

define Build/at91-sdcard
  $(if $(findstring ext4,$@), \
  rm -f $@.boot
  mkfs.fat -C $@.boot $(FAT32_BLOCKS)

  mcopy -i $@.boot \
	$(KDIR)/$(DEVICE_NAME)-fit-zImage.itb \
	::$(DEVICE_NAME)-fit.itb

  $(if $(findstring sama5d27-som1-ek,$@), \
      mcopy -i $@.boot \
          $(BIN_DIR)/u-boot-$(DEVICE_DTS:at91-%=%)_mmc1/u-boot.bin \
          ::u-boot.bin
      mcopy -i $@.boot \
          $(BIN_DIR)/at91bootstrap-$(DEVICE_DTS:at91-%=%)sd1_uboot/at91bootstrap.bin \
          ::BOOT.bin,
      mcopy -i $@.boot \
          $(BIN_DIR)/u-boot-$(DEVICE_DTS:at91-%=%)_mmc/u-boot.bin \
          ::u-boot.bin
      $(if $(findstring sama5d4-xplained,$@), \
          mcopy -i $@.boot \
              $(BIN_DIR)/at91bootstrap-$(DEVICE_DTS:at91-%=%)sd_uboot_secure/at91bootstrap.bin \
              ::BOOT.bin,
          mcopy -i $@.boot \
              $(BIN_DIR)/at91bootstrap-$(DEVICE_DTS:at91-%=%)sd_uboot/at91bootstrap.bin \
              ::BOOT.bin))

  $(CP) uboot-env.txt $@-uboot-env.txt
  sed -i '2d;3d' $@-uboot-env.txt
  sed -i '2i board='"$(DEVICE_NAME)"'' $@-uboot-env.txt
  sed -i '3i board_name='"$(firstword $(SUPPORTED_DEVICES))"'' $@-uboot-env.txt

  mkenvimage -s 0x4000 -o $@-uboot.env $@-uboot-env.txt

  mcopy -i $@.boot $@-uboot.env ::uboot.env

  ./gen_at91_sdcard_img.sh \
	$@.img \
	$@.boot \
	$(KDIR)/root.ext4 \
	$(AT91_SD_BOOT_PARTSIZE) \
	$(CONFIG_TARGET_ROOTFS_PARTSIZE)

  gzip -nc9 $@.img > $@

  rm -f $@.img $@.boot $@-uboot.env $@-uboot-env.txt)
endef

define Device/microchip_sama5d2-xplained
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Microchip
  DEVICE_MODEL := SAMA5D2 Xplained
  DEVICE_DTS := at91-sama5d2_xplained
  SUPPORTED_DEVICES := atmel,sama5d2-xplained
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += microchip_sama5d2-xplained

define Device/microchip_sama5d27-som1-ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Microchip
  DEVICE_MODEL := SAMA5D27 SOM1 Ek
  DEVICE_DTS := at91-sama5d27_som1_ek
  SUPPORTED_DEVICES := atmel,sama5d27-som1-ek
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += microchip_sama5d27-som1-ek

define Device/microchip_sama5d2-ptc-ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Microchip
  DEVICE_MODEL := SAMA5D2 PTC Ek
  DEVICE_DTS := at91-sama5d2_ptc_ek
  SUPPORTED_DEVICES := atmel,sama5d2-ptc_ek
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += microchip_sama5d2-ptc-ek

define Device/microchip_sama5d3-xplained
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Microchip
  DEVICE_MODEL := SAMA5D3 Xplained
  DEVICE_DTS := at91-sama5d3_xplained
  SUPPORTED_DEVICES := atmel,sama5d3-xplained
  KERNEL_SIZE := 6144k
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += microchip_sama5d3-xplained

ifeq ($(strip $(CONFIG_EXTERNAL_KERNEL_TREE)),"")
 ifeq ($(strip $(CONFIG_KERNEL_GIT_CLONE_URI)),"")
  define Device/laird_wb50n
    $(Device/evaluation-fit)
    DEVICE_VENDOR := Laird
    DEVICE_MODEL := WB50N
    DEVICE_PACKAGES := \
	  kmod-mmc-at91 kmod-ath6kl-sdio ath6k-firmware \
	  kmod-usb-storage kmod-fs-vfat kmod-fs-msdos \
	  kmod-leds-gpio
    BLOCKSIZE := 128k
    PAGESIZE := 2048
    SUBPAGESIZE := 2048
    MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 955
  endef
  TARGET_DEVICES += laird_wb50n
 endif
endif

define Device/microchip_sama5d4-xplained
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Microchip
  DEVICE_MODEL := SAMA5D4 Xplained
  DEVICE_DTS := at91-sama5d4_xplained
  SUPPORTED_DEVICES := atmel,sama5d4-xplained
  KERNEL_SIZE := 6144k
  BLOCKSIZE := 256k
  PAGESIZE := 4096
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 248KiB -c 2082
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += microchip_sama5d4-xplained
