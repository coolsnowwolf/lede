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

define Device/FitzImage
	KERNEL_SUFFIX := -fit-zImage.itb
	KERNEL = kernel-bin | fit none $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := zImage
endef

define Device/UbiFit
	KERNEL_IN_UBI := 1
	IMAGES := nand-factory.ubi nand-sysupgrade.bin
	IMAGE/nand-factory.ubi := append-ubi
	IMAGE/nand-sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/netgear_sxr80
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := SXR80
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk01
	SOC := ipq8074
	DEVICE_PACKAGES := ipq-wifi-netgear_sxr80 uboot-envtools
endef
TARGET_DEVICES += netgear_sxr80

define Device/redmi_ax6
	$(call Device/xiaomi_ax3600)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX6
	DEVICE_PACKAGES := ipq-wifi-redmi_ax6 uboot-envtools
endef
TARGET_DEVICES += redmi_ax6

define Device/xiaomi_ax3600
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX3600
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac04
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax3600 kmod-ath10k-ct ath10k-firmware-qca9887-ct \
	uboot-envtools
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
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax9000 kmod-ath10k-ct ath10k-firmware-qca9887-ct \
	uboot-envtools
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
