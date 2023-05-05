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

define Device/tplink_tl-er2260t
	$(call Device/FitImage)
	$(call Device/UbiFit)  
	DEVICE_DTS := ipq8070-tl-er2260t
	DEVICE_DTS_CONFIG := config@hk07
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := tplink,tl-er2260t
	DEVICE_TITLE := TPLINK TL-ER2260T
	DEVICE_PACKAGES := qca-ssdk-shell
endef
TARGET_DEVICES += tplink_tl-er2260t

define Device/tplink_xtr10890
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := TPLINK
	DEVICE_MODEL := XTR10890
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk01.c6
	SOC := ipq8078
	DEVICE_PACKAGES := ipq-wifi-tplink_xtr10890 uboot-envtools
endef
TARGET_DEVICES += tplink_xtr10890

define Device/zyxel_nbg7815
	$(call Device/FitImage)
	DEVICE_VENDOR := ZYXEL
	DEVICE_MODEL := NBG7815
	DEVICE_TITLE := Zyxel Armor G5 (NBG7815)
	DEVICE_DTS_CONFIG := config@nbg7815
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq8074
	IMAGES += factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ipq-wifi-zyxel_nbg7815 kmod-ath11k-pci e2fsprogs kmod-fs-ext4 losetup kmod-hwmon-tmp103
endef
TARGET_DEVICES += zyxel_nbg7815

