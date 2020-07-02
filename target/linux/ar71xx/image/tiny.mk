define Build/mkbuffaloimg
	$(STAGING_DIR_HOST)/bin/mkbuffaloimg -B $(BOARDNAME) \
		-R $$(($(subst k, * 1024,$(ROOTFS_SIZE)))) \
		-K $$(($(subst k, * 1024,$(KERNEL_SIZE)))) \
		-i $@ -o $@.new
	mv $@.new $@
endef


define Device/bhr-4grv2
  DEVICE_TITLE := Buffalo BHR-4GRV2
  BOARDNAME := BHR-4GRV2
  ROOTFS_SIZE := 14528k
  KERNEL_SIZE := 1472k
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | mkbuffaloimg
endef

define Device/zbt-we1526
  DEVICE_TITLE := Zbtlink ZBT-WE1526
  DEVICE_PACKAGES := kmod-usb2
  BOARDNAME := ZBT-WE1526
  IMAGE_SIZE := 16000k
  KERNEL_SIZE := 1472k
  ROOTFS_SIZE := 14528k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
endef
