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

define Device/redmi_ax6
	$(call Device/xiaomi_ax3600)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX6
	DEVICE_PACKAGES := ipq-wifi-redmi_ax6
endef
TARGET_DEVICES += redmi_ax6

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

define Device/xiaomi_ax3600
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX3600
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac04
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax3600 kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9887-ct
endef
TARGET_DEVICES += xiaomi_ax3600

define Device/xiaomi_ax9000
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX9000
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk14
	SOC := ipq8072
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax9000 kmod-ath11k-pci ath11k-firmware-qcn9074 \
	kmod-ath10k-ct ath10k-firmware-qca9887-ct
endef
TARGET_DEVICES += xiaomi_ax9000

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

