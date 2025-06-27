define Device/EmmcImage
	IMAGES += factory.bin recovery.bin
	IMAGE/factory.bin := append-kernel | pad-to 12288k | append-rootfs | append-metadata
	IMAGE/recovery.bin := append-kernel | pad-to 6144k | append-rootfs | append-metadata
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/glinet_gl-b3000
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-B3000
	SOC := ipq5018
	KERNEL_IN_UBI := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	NAND_SIZE := 128m
	DEVICE_DTS_CONFIG := config@mp03.5-c1
	SUPPORTED_DEVICES += b3000
	DEVICE_PACKAGES := \
		ath11k-firmware-qcn6122 \
		ipq-wifi-glinet_gl-b3000
endef
TARGET_DEVICES += glinet_gl-b3000

define Device/jdcloud_re-cs-03
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	SOC := ipq5018
	BLOCKSIZE := 64k
	KERNEL_SIZE := 6144k
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := AX3000
	DEVICE_DTS_CONFIG := config@mp03.5-c2
endef
TARGET_DEVICES += jdcloud_re-cs-03

define Device/linksys_ipq50xx_mx_base
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Linksys
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_SIZE := 8192k
	IMAGE_SIZE := 83968k
	NAND_SIZE := 256m
	SOC := ipq5018
	IMAGES += factory.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
		append-ubi | linksys-image type=$$$$(DEVICE_MODEL)
endef

define Device/linksys_mr5500
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MR5500
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := kmod-ath11k-pci \
		ath11k-firmware-qcn9074 \
		ipq-wifi-linksys_mr5500 \
		kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_mr5500

define Device/linksys_mx2000
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MX2000
	DEVICE_DTS_CONFIG := config@mp03.5-c1
	DEVICE_PACKAGES := ath11k-firmware-qcn6122 \
		ipq-wifi-linksys_mx2000
endef
TARGET_DEVICES += linksys_mx2000

define Device/linksys_mx5500
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MX5500
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := kmod-ath11k-pci \
		ath11k-firmware-qcn9074 \
		ipq-wifi-linksys_mx5500
endef
TARGET_DEVICES += linksys_mx5500

define Device/linksys_spnmx56
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := SPNMX56
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := kmod-ath11k-pci \
		ath11k-firmware-qcn9074 \
		ipq-wifi-linksys_spnmx56
endef
TARGET_DEVICES += linksys_spnmx56

define Device/xiaomi_ax6000
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX6000
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@mp03.1
	SOC := ipq5018
	KERNEL_SIZE := 36864k
	NAND_SIZE := 128m
	DEVICE_PACKAGES := kmod-ath11k-pci \
		ath11k-firmware-qcn9074 \
		kmod-ath10k kmod-ath10k-smallbuffers \
		ath10k-firmware-qca9887 \
		ipq-wifi-xiaomi_ax6000
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	ARTIFACTS := initramfs-factory.ubi
	ARTIFACT/initramfs-factory.ubi := append-image-stage initramfs-uImage.itb | ubinize-kernel
endif
endef
TARGET_DEVICES += xiaomi_ax6000
