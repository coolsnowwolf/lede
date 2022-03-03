KERNEL_LOADADDR := 0x80008000
DEVICE_VARS += UBOOT_TARGET UBOOT_OFFSET UBOOT_ENVSIZE

# The bootrom of MT7623 expects legacy MediaTek headers present in
# exactly the location also used for the primary GPT partition table.
# (*MMC_BOOT and BRLYT)
# Hence only MSDOS/MBR partitioning can work here.
#
#   ------------------------   Sector   Offset
#   |  MBR + SDMMC_BOOT    |     0       0x0
#   |----------------------|
#   |     BRLYT header     |     1       0x200
#   |----------------------|
#   .                      .
#   .                      .
#   |----------------------|
#   |                      |     4       0x800
#   |                      |
#   |     Preloader        |
#   .                      .
#   .                      .
#   |                      |     639
#   |----------------------|
#   |   MBR partition #1   |     640     0x50000
#   |                      |
#   |       U-Boot         |
#   .                      .
#   .                      .
#   |                      |     1663
#   |----------------------|
#   |   MBR partition #2   |
#   |                      |
#   |       Recovery       |
#   .                      .
#   .     (uImage.FIT)     .
#   |                      |
#   |----------------------|
#   |   MBR partition #3   |
#   |                      |
#   |      Production      |
#   |                      |
#   |     (uImage.FIT,     |
#   .     rootfs_Data.)    .
#   .                      .
#   |                      |
#   ------------------------
#
# For eMMC boot, everything up to and including the preloader must be
# written to /dev/mmcblk0boot0, with the SDMMC_BOOT header changed to
# read EMMC_BOOT\0 instead.
#
# The contents of the main eMMC are identical to the SD card layout,
# with the preloader loading 512KiB of U-Boot starting at 0x50000.

define Build/mt7623-mbr
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -o $@.tmp -h 4 -s 63 -a 0 -l 1024 \
			-t 0x41	-N uboot	-p 3584k@320k \
			-t 0xea	-N recovery	-p 40M@4M \
		$(if $(findstring sdmmc,$1), \
			-t 0x2e -N production	-p 208M@48M \
		) \
		$(if $(findstring emmc,$1), \
			-t 0x2e -N production	-p 976M@48M \
		)

	echo -en \
		$(if $(findstring sdmmc,$1),"SDMMC_BOOT\x00\x00\x01\x00\x00\x00\x00\x02\x00\x00") \
		$(if $(findstring emmc,$1),"EMMC_BOOT\x00\x00\x00\x01\x00\x00\x00\x00\x02\x00\x00") \
		| dd bs=1 of="$@.tmp" seek=0 conv=notrunc

	echo -en "BRLYT\x00\x00\x00\x01\x00\x00\x00\x00\x08\x00\x00\x00\x08\x00\x00\x42\x42\x42\x42\x08\x00\x01\x00\x00\x08\x00\x00\x00\x08\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
		| dd bs=1 of="$@.tmp" seek=512 conv=notrunc

	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/append-preloader
	cat $(STAGING_DIR_IMAGE)/$1-preloader.bin >> $@
endef

define Build/append-bootloader
	cat $(STAGING_DIR_IMAGE)/$1-u-boot.bin >> $@
endef

define Device/bananapi_bpi-r2
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R2
  DEVICE_DTS := mt7623n-bananapi-bpi-r2
  DEVICE_PACKAGES := mkf2fs e2fsprogs kmod-usb3 kmod-ata-ahci
  UBOOT_ENVSIZE := 0x10000
  UBOOT_OFFSET := 320k
  UBOOT_TARGET := mt7623n_bpir2
  UBOOT_PATH := $(STAGING_DIR_IMAGE)/mt7623n_bpir2-u-boot.bin
  IMAGES := sysupgrade.itb
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb with-initrd
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb external-static-with-rootfs | append-metadata
  ARTIFACT/preloader.bin := append-preloader $$(UBOOT_TARGET)
  ARTIFACT/u-boot.bin := append-uboot
  ARTIFACT/sdcard.img.gz := mt7623-mbr sdmmc |\
			    pad-to 2k | append-preloader $$(UBOOT_TARGET) |\
			    pad-to 320k | append-bootloader $$(UBOOT_TARGET) |\
			    pad-to 4092k | mt7623-mbr emmc |\
			    pad-to 4M | append-image-stage initramfs-recovery.itb |\
			    pad-to 48M | append-image squashfs-sysupgrade.itb |\
			    gzip
  ARTIFACTS = u-boot.bin preloader.bin scatter.txt sdcard.img.gz
  SUPPORTED_DEVICES := bananapi,bpi-r2
endef
TARGET_DEVICES += bananapi_bpi-r2


ifneq ($(CONFIG_MTK_BOOT_PARTSIZE),)
BOOTFS_BLOCK_SIZE := 1024
BOOTFS_BLOCKS := $(shell echo $$(($(CONFIG_MTK_BOOT_PARTSIZE)*1024*1024/$(BOOTFS_BLOCK_SIZE))))
endif

define Build/mtk-mmc-img
	rm -f $@.boot
	mkfs.fat -C $@.boot $(BOOTFS_BLOCKS)

	if [ -r $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin ]; then \
		./gen_mtk_mmc_img.sh emmc $@.emmc \
			$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin; \
		mcopy -i $@.boot $@.emmc ::eMMCboot.bin; \
	fi
	mkenvimage -s $(UBOOT_ENVSIZE) -o $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-uboot.env $(UBOOT_TARGET)-uEnv.txt
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-uboot.env ::uboot.env
	mcopy -i $@.boot $(IMAGE_KERNEL) ::uImage
	./gen_mtk_mmc_img.sh sd $@ \
		$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-preloader.bin \
		$(STAGING_DIR_IMAGE)/$(UBOOT_TARGET)-u-boot*.bin \
		$(UBOOT_OFFSET) \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_MTK_BOOT_PARTSIZE) \
		104
endef

define Build/scatterfile
	./gen_scatterfile.sh $(subst mt,MT,$(SUBTARGET)) "$1" \
		$(subst -scatter.txt,,$(notdir $@)) "$(DEVICE_TITLE)" > $@
endef


# Full eMMC image including U-Boot and partition table
define Device/unielec_u7623-emmc
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7623
  DEVICE_VARIANT := eMMC
  # When we use FIT images, U-Boot will populate the /memory node with the correct
  # memory size discovered from the preloader, so we don't need separate builds.
  DEVICE_DTS := mt7623a-unielec-u7623-02-emmc-512m
  DEVICE_DTS_DIR := ../dts
  SUPPORTED_DEVICES := unielec,u7623-02-emmc-512m
  UBOOT_ENVSIZE := 0x1000
  UBOOT_OFFSET := 256k
  UBOOT_TARGET := mt7623a_unielec_u7623
  IMAGES := img.gz
  IMAGE/img.gz := mtk-mmc-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc \
       mkf2fs e2fsprogs kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk
  ARTIFACT/scatter.txt := scatterfile $$(firstword $$(FILESYSTEMS))-$$(firstword $$(IMAGES))
  ARTIFACTS := scatter.txt
endef
TARGET_DEVICES += unielec_u7623-emmc

# Legacy partial image for U7623
# This preserves the vendor U-Boot and starts with a uImage at 0xA00
define Device/unielec_u7623-02-emmc-512m-legacy
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7623-02
  DEVICE_VARIANT := eMMC/512MiB RAM (legacy image)
  DEVICE_DTS := mt7623a-unielec-u7623-02-emmc-512m
  DEVICE_DTS_DIR := ../dts
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_INITRAMFS := kernel-bin | append-dtb | uImage none
  DEVICE_PACKAGES := kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc \
	mkf2fs e2fsprogs kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk
  IMAGES := sysupgrade-emmc.bin.gz
  IMAGE/sysupgrade-emmc.bin.gz := sysupgrade-emmc | gzip | append-metadata
  SUPPORTED_DEVICES := unielec,u7623-02-emmc-512m
endef
TARGET_DEVICES += unielec_u7623-02-emmc-512m-legacy
