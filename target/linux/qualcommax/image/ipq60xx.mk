define Device/EmmcImage
	IMAGES += factory.bin recovery.bin
	IMAGE/factory.bin := append-kernel | pad-to 12288k | append-rootfs | append-metadata
	IMAGE/recovery.bin := append-kernel | pad-to 6144k | append-rootfs | append-metadata
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/cmiot_ax18
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := CMIOT
	DEVICE_MODEL := AX18
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@cp03-c1
	SOC := ipq6000
endef
TARGET_DEVICES += cmiot_ax18

define Device/glinet_gl-common
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := GL.iNet
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@cp03-c1
	SOC := ipq6000
endef

define Device/glinet_gl-ax1800
	$(call Device/glinet_gl-common)
	DEVICE_MODEL := GL-AX1800
	DEVICE_PACKAGES := ipq-wifi-glinet_gl-ax1800
	SUPPORTED_DEVICES += glinet,ax1800
endef
TARGET_DEVICES += glinet_gl-ax1800

define Device/glinet_gl-axt1800
	$(call Device/glinet_gl-common)
	DEVICE_MODEL := GL-AXT1800
	DEVICE_PACKAGES := ipq-wifi-glinet_gl-axt1800 kmod-hwmon-pwmfan
	SUPPORTED_DEVICES += glinet,axt1800
endef
TARGET_DEVICES += glinet_gl-axt1800

define Device/jdcloud_re-cs-02
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := AX6600
	SOC := ipq6010
	BLOCKSIZE := 64k
	KERNEL_SIZE := 6144k
	DEVICE_DTS_CONFIG := config@cp03-c3
	DEVICE_PACKAGES := ipq-wifi-jdcloud_re-cs-02 kmod-ath11k-pci ath11k-firmware-qcn9074
endef
TARGET_DEVICES += jdcloud_re-cs-02

define Device/jdcloud_re-cs-07
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := ER1
	SOC := ipq6010
	BLOCKSIZE := 64k
	KERNEL_SIZE := 6144k
	DEVICE_DTS_CONFIG := config@cp03-c4
	DEVICE_PACKAGES := -kmod-ath11k-ahb -ath11k-firmware-ipq6018
endef
TARGET_DEVICES += jdcloud_re-cs-07

define Device/jdcloud_re-ss-01
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := AX1800 Pro
	SOC := ipq6000
	BLOCKSIZE := 64k
	KERNEL_SIZE := 6144k
	DEVICE_DTS_CONFIG := config@cp03-c2
	DEVICE_PACKAGES := ipq-wifi-jdcloud_ax1800pro
endef
TARGET_DEVICES += jdcloud_re-ss-01

define Device/linksys_mr7350
	$(call Device/FitImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := MR7350
	SOC := ipq6000
	KERNEL_SIZE := 8192k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	UBINIZE_OPTS := -E 5
	IMAGES += factory.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
		append-ubi | linksys-image type=MR7350
	DEVICE_PACKAGES := ipq-wifi-linksys_mr7350 \
		kmod-leds-pca963x kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_mr7350

define Device/qihoo_360v6
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qihoo
	DEVICE_MODEL := 360V6
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-qihoo_360v6
endef
TARGET_DEVICES += qihoo_360v6

define Device/redmi_ax5-jdcloud
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX5 JDCloud
	DEVICE_DTS_CONFIG := config@cp03-c1
	SOC := ipq6000
	DEVICE_PACKAGES := ipq-wifi-redmi_ax5-jdcloud
endef
TARGET_DEVICES += redmi_ax5-jdcloud

define Device/xiaomi_rm1800
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := RM1800 (AX5)
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-xiaomi_rm1800
endef
TARGET_DEVICES += xiaomi_rm1800
