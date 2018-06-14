define Build/senao-factory-image
	mkdir -p $@.senao

	touch $@.senao/FWINFO-OpenWrt-$(REVISION)-$(1)
	$(CP) $(IMAGE_KERNEL) $@.senao/openwrt-senao-$(1)-uImage-lzma.bin
	$(CP) $@ $@.senao/openwrt-senao-$(1)-root.squashfs

	$(TAR) -c \
		--numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-C $@.senao . | gzip -9nc > $@

	rm -rf $@.senao
endef


define Device/ens202ext
  DEVICE_TITLE := EnGenius ENS202EXT
  BOARDNAME := ENS202EXT
  DEVICE_PACKAGES := rssileds
  KERNEL_SIZE := 1536k
  IMAGE_SIZE := 13632k
  IMAGES += factory.bin
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),320k(custom)ro,1536k(kernel),12096k(rootfs),2048k(failsafe)ro,64k(art)ro,13632k@0xa0000(firmware)
  IMAGE/factory.bin := append-rootfs | pad-rootfs | senao-factory-image ens202ext
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ens202ext

define Device/koala
  DEVICE_TITLE := OCEDO Koala
  BOARDNAME := KOALA
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  KERNEL_SIZE := 1536k
  IMAGE_SIZE := 7424k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),1536k(kernel),5888k(rootfs),1536k(kernel2),5888k(rootfs2),1088k(data)ro,64k(id)ro,64k(art)ro,7424k@0x50000(firmware)
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += koala
