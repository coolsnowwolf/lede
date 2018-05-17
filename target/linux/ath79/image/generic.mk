define Build/netgear-squashfs
	rm -rf $@.fs $@.squashfs
	mkdir -p $@.fs/image
	cp $@ $@.fs/image/uImage
	$(STAGING_DIR_HOST)/bin/mksquashfs-lzma \
		$@.fs $@.squashfs \
		-noappend -root-owned -be -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))

	dd if=/dev/zero bs=1k count=1 >> $@.squashfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		-M $(NETGEAR_KERNEL_MAGIC) \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d $@.squashfs $@
	rm -rf $@.squashfs $@.fs
endef

define Build/netgear-uImage
	$(call Build/uImage,$(1) -M $(NETGEAR_KERNEL_MAGIC))
endef

define Device/ew_dorin
  ATH_SOC := ar9331
  DEVICE_TITLE := Embedded Wireless Dorin
  DEVICE_PACKAGES := kmod-usb-chipidea2
endef

TARGET_DEVICES += ew_dorin

define Device/gl_ar150
  ATH_SOC := ar9330
  DEVICE_TITLE := GL.iNet GL-AR150
  DEVICE_PACKAGES := kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += gl_ar150

define Device/om5p_ac
  ATH_SOC := qca9558
  DEVICE_TITLE := OpenMesh OM5P-AC
  DEVICE_PROFILE := OM5P-AC
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x om-watchdog
endef

TARGET_DEVICES += om5p_ac

define Device/netgear_wndr3700
  ATH_SOC:=ar7100
  DEVICE_TITLE := NETGEAR WNDR3700
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-reset
  NETGEAR_KERNEL_MAGIC := 0x33373030
  NETGEAR_BOARD_ID := WNDR3700
  IMAGE_SIZE := 7680k
  IMAGES := sysupgrade.bin factory.img factory-NA.img
  KERNEL := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | check-size $$$$(IMAGE_SIZE)
endef
#TARGET_DEVICES += netgear_wndr3700

define Device/netgear_wndr3700v2
  $(Device/netgear_wndr3700)
  DEVICE_TITLE := NETGEAR WNDR3700 v2
  NETGEAR_BOARD_ID := WNDR3700v2
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_HW_ID := 29763654+16+64
  IMAGE_SIZE := 15872k
  IMAGES := sysupgrade.bin factory.img
endef
#TARGET_DEVICES += netgear_wndr3700v2

define Device/netgear_wndr3800
  $(Device/netgear_wndr3700v2)
  ATH_SOC := ar7161
  DEVICE_TITLE := NETGEAR WNDR3800
  NETGEAR_BOARD_ID := WNDR3800
  NETGEAR_HW_ID := 29763654+16+128
endef
TARGET_DEVICES += netgear_wndr3800

define Device/buffalo_wzr-hp-g450h
  ATH_SOC := ar7242
  DEVICE_TITLE := Buffalo WZR-HP-G450H
  DEVICE_PROFILE := WZR-HP-G450H
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef

TARGET_DEVICES += buffalo_wzr-hp-g450h
