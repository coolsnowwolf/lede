define Build/hdd-img
	./mbl_gen_hdd_img.sh $@ $@.boot $(IMAGE_ROOTFS) $(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef


define Device/wd_mybooklive
  DEVICE_VENDOR := Western Digital
  DEVICE_MODEL := My Book Live Series (Single + Duo)
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-fs-vfat wpad-basic
  DEVICE_DTS := wd-mybooklive
  SUPPORTED_DEVICES += mbl wd,mybooklive-duo
  BLOCKSIZE := 1k
  DTB_SIZE := 16384
  KERNEL := kernel-bin | dtb | gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | gzip | dtb | MuImage-initramfs gzip
  IMAGES := factory.img.gz sysupgrade.img.gz
  ARTIFACTS := apollo3g.dtb
  DEVICE_DTB := apollo3g.dtb
  FILESYSTEMS := ext4 squashfs
  IMAGE/factory.img.gz := boot-script | boot-img | hdd-img | gzip
  IMAGE/sysupgrade.img.gz := boot-script | boot-img | hdd-img | gzip | append-metadata
  ARTIFACT/apollo3g.dtb := export-dtb
endef

TARGET_DEVICES += wd_mybooklive
