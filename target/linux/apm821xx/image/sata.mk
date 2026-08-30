define Build/hdd-img
	./mbl_gen_hdd_img.sh $@ $@.boot $(IMAGE_ROOTFS) $(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef


define Device/wd_mybooklive
  DEVICE_VENDOR := Western Digital
  DEVICE_MODEL := My Book Live
  DEVICE_ALT0_VENDOR := Western Digital
  DEVICE_ALT0_MODEL := My Book Live Duo
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-ata-dwc kmod-usb-ledtrig-usbport \
	kmod-usb-storage kmod-fs-vfat wpad-basic-mbedtls
  SUPPORTED_DEVICES += mbl wd,mybooklive-duo
  BLOCKSIZE := 1k
  DEVICE_DTC_FLAGS := --pad 4096
  KERNEL := kernel-bin | libdeflate-gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | libdeflate-gzip | MuImage-initramfs gzip
  IMAGES := factory.img.gz sysupgrade.img.gz
  ARTIFACTS := apollo3g.dtb
  DEVICE_DTB := apollo3g.dtb
  FILESYSTEMS := ext4 squashfs
  IMAGE/factory.img.gz := boot-script | boot-img | hdd-img | libdeflate-gzip
  IMAGE/sysupgrade.img.gz := boot-script | boot-img | hdd-img | libdeflate-gzip | append-metadata
  ARTIFACT/apollo3g.dtb := export-dtb
endef

TARGET_DEVICES += wd_mybooklive
