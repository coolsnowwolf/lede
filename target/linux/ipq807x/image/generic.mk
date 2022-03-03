define Device/FitImage
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/FitImageLzma
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | lzma | fit lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/UbiFit
	KERNEL_IN_UBI := 1
	IMAGES := nand-factory.ubi nand-sysupgrade.bin
	IMAGE/nand-factory.ubi := append-ubi
	IMAGE/nand-sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/qnap_301w
	$(call Device/FitImage)
	DEVICE_VENDOR := QNAP
	DEVICE_MODEL := 301w
	DEVICE_DTS_CONFIG := config@hk01
	KERNEL_SIZE := 16384k
	BLOCKSIZE := 512k
	SOC := ipq8072
	IMAGES += factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ipq-wifi-qnap_301w e2fsprogs kmod-fs-ext4 losetup
endef
TARGET_DEVICES += qnap_301w

define Device/zte_mf269
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := ZTE
	DEVICE_MODEL := MF269
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac04
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-zte_mf269 uboot-envtools
endef
TARGET_DEVICES += zte_mf269
