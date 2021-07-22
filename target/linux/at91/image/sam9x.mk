
define Device/default-nand
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 126KiB -c 2048
endef

define Build/at91-sdcard
  $(if $(findstring ext4,$@), \
  rm -f $@.boot
  mkfs.fat -C $@.boot $(FAT32_BLOCKS)

  mcopy -i $@.boot \
	$(KDIR)/$(DEVICE_NAME)-fit-zImage.itb \
	::$(DEVICE_NAME)-fit.itb

  mcopy -i $@.boot \
	$(BIN_DIR)/u-boot-at91sam9x5ek_mmc/u-boot.bin \
	::u-boot.bin

  mcopy -i $@.boot \
	$(BIN_DIR)/at91bootstrap-at91sam9x5eksd_uboot/at91bootstrap.bin \
	::BOOT.bin

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

define Device/atmel_at91sam9263ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9263-EK
endef
TARGET_DEVICES += atmel_at91sam9263ek

define Device/atmel_at91sam9g15ek
  $(Device/evaluation)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9G15-EK
endef
TARGET_DEVICES += atmel_at91sam9g15ek

define Device/atmel_at91sam9g20ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9G20-EK
endef
TARGET_DEVICES += atmel_at91sam9g20ek

define Device/atmel_at91sam9g20ek-2mmc
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9G20-EK
  DEVICE_VARIANT := 2MMC
  DEVICE_DTS := at91sam9g20ek_2mmc
  SUPPORTED_DEVICES := atmel,at91sam9g20ek_2mmc
endef
TARGET_DEVICES += atmel_at91sam9g20ek-2mmc

define Device/atmel_at91sam9g25ek
  $(Device/evaluation)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9G25-EK
endef
TARGET_DEVICES += atmel_at91sam9g25ek

define Device/atmel_at91sam9g35ek
  $(Device/evaluation)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9G35-EK
endef
TARGET_DEVICES += atmel_at91sam9g35ek

define Device/atmel_at91sam9m10g45ek
  $(Device/evaluation)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9M10G45-EK
endef
TARGET_DEVICES += atmel_at91sam9m10g45ek

define Device/atmel_at91sam9x25ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9X25-EK
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += atmel_at91sam9x25ek

define Device/atmel_at91sam9x35ek
  $(Device/evaluation-dtb)
  DEVICE_VENDOR := Atmel
  DEVICE_MODEL := AT91SAM9X35-EK
  $(Device/evaluation-sdimage)
endef
TARGET_DEVICES += atmel_at91sam9x35ek

define Device/calamp_lmu5000
  $(Device/production)
  DEVICE_VENDOR := CalAmp
  DEVICE_MODEL := LMU5000
  DEVICE_PACKAGES := kmod-rtc-pcf2123 kmod-usb-acm \
	kmod-usb-serial-option kmod-usb-serial-sierrawireless kmod-gpio-mcp23s08
endef
TARGET_DEVICES += calamp_lmu5000

define Device/calao_tny-a9260
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := TNY A9260
  DEVICE_DTS := tny_a9260
endef
TARGET_DEVICES += calao_tny-a9260

define Device/calao_tny-a9263
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := TNY A9263
  DEVICE_DTS := tny_a9263
  SUPPORTED_DEVICES := atmel,tny-a9263
endef
TARGET_DEVICES += calao_tny-a9263

define Device/calao_tny-a9g20
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := TNY A9G20
  DEVICE_DTS := tny_a9g20
endef
TARGET_DEVICES += calao_tny-a9g20

define Device/calao_usb-a9260
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := USB A9260
  DEVICE_DTS := usb_a9260
endef
TARGET_DEVICES += calao_usb-a9260

define Device/calao_usb-a9263
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := USB A9263
  DEVICE_DTS := usb_a9263
  SUPPORTED_DEVICES := atmel,usb-a9263
endef
TARGET_DEVICES += calao_usb-a9263

define Device/calao_usb-a9g20
  $(Device/production-dtb)
  DEVICE_VENDOR := Calao
  DEVICE_MODEL := USB A9G20
  DEVICE_DTS := usb_a9g20
endef
TARGET_DEVICES += calao_usb-a9g20

define Device/egnite_ethernut5
  $(Device/evaluation)
  DEVICE_VENDOR := egnite
  DEVICE_MODEL := Ethernut 5
  UBINIZE_OPTS := -E 5
endef
TARGET_DEVICES += egnite_ethernut5

define Device/exegin_q5xr5
  $(Device/production-dtb)
  DEVICE_VENDOR := Exegin
  DEVICE_MODEL := Q5x
  DEVICE_VARIANT := rev5
  DEVICE_DTS := at91-q5xr5
  KERNEL_SIZE := 2048k
  DEFAULT := n
endef
TARGET_DEVICES += exegin_q5xr5

define Device/laird_wb45n
  $(Device/evaluation-fit)
  DEVICE_VENDOR := Laird
  DEVICE_MODEL := WB45N
  DEVICE_PACKAGES := \
	kmod-mmc-at91 kmod-ath6kl-sdio ath6k-firmware \
	kmod-usb-storage kmod-fs-vfat kmod-fs-msdos \
	kmod-leds-gpio
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 955
endef
TARGET_DEVICES += laird_wb45n
