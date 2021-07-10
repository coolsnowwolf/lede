# attention: only zlib compression is allowed for the boot fs
define Build/zyxel-buildkerneljffs
	rm -rf  $(KDIR_TMP)/zyxelnbg6716
	mkdir -p $(KDIR_TMP)/zyxelnbg6716/image/boot
	cp $@ $(KDIR_TMP)/zyxelnbg6716/image/boot/vmlinux.lzma.uImage
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		--big-endian --squash-uids -v -e 128KiB -q -f -n -x lzma -x rtime \
		-o $@ \
		-d $(KDIR_TMP)/zyxelnbg6716/image
	rm -rf $(KDIR_TMP)/zyxelnbg6716
endef

define Build/zyxel-factory
	let \
		maxsize="$(subst k,* 1024,$(RAS_ROOTFS_SIZE))"; \
		let size="$$(stat -c%s $@)"; \
		if [ $$size -lt $$maxsize ]; then \
			$(STAGING_DIR_HOST)/bin/mkrasimage \
				-b $(RAS_BOARD) \
				-v $(RAS_VERSION) \
				-r $@ \
				-s $$maxsize \
				-o $@.new \
				-l 131072 \
			&& mv $@.new $@ ; \
		fi
endef

define Device/8dev_rambutan
  SOC := qca9557
  DEVICE_VENDOR := 8devices
  DEVICE_MODEL := Rambutan
  DEVICE_PACKAGES := kmod-usb2
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  KERNEL_IN_UBI := 1
  IMAGES := factory.bin sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-ubi
endef
TARGET_DEVICES += 8dev_rambutan

define Device/aerohive_hiveap-121
  SOC := ar9344
  DEVICE_VENDOR := Aerohive
  DEVICE_MODEL := HiveAP 121
  DEVICE_PACKAGES := kmod-usb2
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 116m
  KERNEL_SIZE := 5120k
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += hiveap-121
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
	check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += aerohive_hiveap-121

define Device/arris_sbr-ac1750
  SOC := qca9558
  DEVICE_VENDOR := Arris
  DEVICE_MODEL := SBR-AC1750
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  IMAGE_SIZE := 32m
  PAGESIZE := 2048
  KERNEL := kernel-bin | append-dtb | gzip | uImage gzip
  KERNEL_INITRAMFS := kernel-bin | append-dtb | uImage none
  IMAGES += kernel1.bin rootfs1.bin factory-kfs.bin
  IMAGE/kernel1.bin := append-kernel | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs1.bin := append-ubi | check-size
  IMAGE/factory-kfs.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
endef
TARGET_DEVICES += arris_sbr-ac1750

define Device/domywifi_dw33d
  SOC := qca9558
  DEVICE_VENDOR := DomyWifi
  DEVICE_MODEL := DW33D
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-storage kmod-usb-ledtrig-usbport \
	kmod-ath10k-ct ath10k-firmware-qca988x-ct
  KERNEL_SIZE := 5120k
  IMAGE_SIZE := 98304k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
	check-size
endef
TARGET_DEVICES += domywifi_dw33d

define Device/domywifi_dw33d-nor
  $(Device/domywifi_dw33d)
  DEVICE_VARIANT := NOR
  IMAGE_SIZE := 14464k
  BLOCKSIZE := 64k
  LOADER_TYPE := bin
  LOADER_FLASH_OFFS := 0x60000
  COMPILE := loader-$(1).bin loader-$(1).uImage
  COMPILE/loader-$(1).bin := loader-okli-compile
  COMPILE/loader-$(1).uImage := append-loader-okli $(1) | pad-to 64k | lzma | uImage lzma
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x4f4b4c49
  IMAGES := sysupgrade.bin breed-factory.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
			  append-metadata | check-size
  IMAGE/breed-factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
			     prepad-okli-kernel $(1) | pad-to 14528k | append-okli-kernel $(1)
endef
TARGET_DEVICES += domywifi_dw33d-nor

define Device/glinet_gl-ar300m-common-nand
  SOC := qca9531
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-AR300M
  DEVICE_PACKAGES := kmod-usb2
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 16000k
  PAGESIZE := 2048
  VID_HDR_OFFSET := 2048
endef

define Device/glinet_gl-ar300m-nand
  $(Device/glinet_gl-ar300m-common-nand)
  DEVICE_VARIANT := NAND
  BLOCKSIZE := 128k
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  SUPPORTED_DEVICES += glinet,gl-ar300m-nor
endef
TARGET_DEVICES += glinet_gl-ar300m-nand

define Device/glinet_gl-ar300m-nor
  $(Device/glinet_gl-ar300m-common-nand)
  DEVICE_VARIANT := NOR
  BLOCKSIZE := 64k
  SUPPORTED_DEVICES += glinet,gl-ar300m-nand gl-ar300m
endef
TARGET_DEVICES += glinet_gl-ar300m-nor

define Device/glinet_gl-ar750s-common
  SOC := qca9563
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-AR750S
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9887-ct kmod-usb2 \
	kmod-usb-storage block-mount
  IMAGE_SIZE := 16000k
endef

define Device/glinet_gl-ar750s-nor-nand
  $(Device/glinet_gl-ar750s-common)
  DEVICE_VARIANT := NOR/NAND
  KERNEL_SIZE := 4096k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  SUPPORTED_DEVICES += glinet,gl-ar750s-nor
endef
TARGET_DEVICES += glinet_gl-ar750s-nor-nand

define Device/glinet_gl-ar750s-nor
  $(Device/glinet_gl-ar750s-common)
  DEVICE_VARIANT := NOR
  BLOCKSIZE := 64k
  SUPPORTED_DEVICES += gl-ar750s glinet,gl-ar750s glinet,gl-ar750s-nor-nand
endef
TARGET_DEVICES += glinet_gl-ar750s-nor

define Device/glinet_gl-e750
  SOC := qca9531
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-E750
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9887-ct kmod-usb2
  SUPPORTED_DEVICES += gl-e750
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 131072k
  PAGESIZE := 2048
  VID_HDR_OFFSET := 2048
  BLOCKSIZE := 128k
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-e750

# fake rootfs is mandatory, pad-offset 129 equals (2 * uimage_header + 0xff)
define Device/netgear_ath79_nand
  DEVICE_VENDOR := NETGEAR
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 25600k
  KERNEL := kernel-bin | append-dtb | lzma -d20 | \
	pad-offset $$(KERNEL_SIZE) 129 | uImage lzma | \
	append-string -e '\xff' | \
	append-uImage-fakehdr filesystem $$(UIMAGE_MAGIC)
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | uImage lzma
  IMAGES := sysupgrade.bin factory.img
  IMAGE/factory.img := append-kernel | append-ubi | netgear-dni | \
	check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata | \
	check-size
  UBINIZE_OPTS := -E 5
endef

define Device/netgear_wndr3700-v4
  SOC := ar9344
  DEVICE_MODEL := WNDR3700
  DEVICE_VARIANT := v4
  UIMAGE_MAGIC := 0x33373033
  NETGEAR_BOARD_ID := WNDR3700v4
  NETGEAR_HW_ID := 29763948+128+128
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr3700-v4

define Device/netgear_wndr4300
  SOC := ar9344
  DEVICE_MODEL := WNDR4300
  UIMAGE_MAGIC := 0x33373033
  NETGEAR_BOARD_ID := WNDR4300
  NETGEAR_HW_ID := 29763948+0+128+128+2x2+3x3
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr4300

define Device/netgear_wndr4300sw
  SOC := ar9344
  DEVICE_MODEL := WNDR4300SW
  UIMAGE_MAGIC := 0x33373033
  NETGEAR_BOARD_ID := WNDR4300SW
  NETGEAR_HW_ID := 29763948+0+128+128+2x2+3x3
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr4300sw

define Device/netgear_wndr4300tn
  SOC := ar9344
  DEVICE_MODEL := WNDR4300TN
  UIMAGE_MAGIC := 0x33373033
  NETGEAR_BOARD_ID := WNDR4300TN
  NETGEAR_HW_ID := 29763948+0+128+128+2x2+3x3
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr4300tn

define Device/netgear_wndr4300-v2
  SOC := qca9563
  DEVICE_MODEL := WNDR4300
  DEVICE_VARIANT := v2
  UIMAGE_MAGIC := 0x27051956
  NETGEAR_BOARD_ID := WNDR4500series
  NETGEAR_HW_ID := 29764821+2+128+128+3x3+3x3+5508012175
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr4300-v2

define Device/netgear_wndr4500-v3
  SOC := qca9563
  DEVICE_MODEL := WNDR4500
  DEVICE_VARIANT := v3
  UIMAGE_MAGIC := 0x27051956
  NETGEAR_BOARD_ID := WNDR4500series
  NETGEAR_HW_ID := 29764821+2+128+128+3x3+3x3+5508012173
  $(Device/netgear_ath79_nand)
endef
TARGET_DEVICES += netgear_wndr4500-v3

define Device/zyxel_nbg6716
  SOC := qca9558
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NBG6716
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  RAS_BOARD := NBG6716
  RAS_ROOTFS_SIZE := 29696k
  RAS_VERSION := "OpenWrt Linux-$(LINUX_VERSION)"
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL := kernel-bin | append-dtb | uImage none | zyxel-buildkerneljffs | \
	check-size 4096k
  IMAGES := sysupgrade.tar sysupgrade-4M-Kernel.bin factory.bin
  IMAGE/sysupgrade.tar/squashfs := append-rootfs | pad-to $$$$(BLOCKSIZE) | \
	sysupgrade-tar rootfs=$$$$@ | append-metadata
  IMAGE/sysupgrade-4M-Kernel.bin/squashfs := append-kernel | \
	pad-to $$$$(KERNEL_SIZE) | append-ubi | pad-to 263192576 | gzip
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
	zyxel-factory
  UBINIZE_OPTS := -E 5
  DEFAULT := n
endef
TARGET_DEVICES += zyxel_nbg6716
