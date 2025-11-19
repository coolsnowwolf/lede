define Device/EmmcImage
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Build/wax6xx-netgear-tar
	mkdir $@.tmp
	mv $@ $@.tmp/nand-ipq807x-apps.img
	md5sum $@.tmp/nand-ipq807x-apps.img | cut -c 1-32 > $@.tmp/nand-ipq807x-apps.md5sum
	echo $(DEVICE_MODEL) > $@.tmp/metadata.txt
	echo $(DEVICE_MODEL)"_V9.9.9.9" > $@.tmp/version
	tar -C $@.tmp/ -cf $@ .
	rm -rf $@.tmp
endef

define Device/aliyun_ap8220
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Aliyun
	DEVICE_MODEL := AP8220
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac02
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-aliyun_ap8220
endef
TARGET_DEVICES += aliyun_ap8220

define Device/arcadyan_aw1000
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Arcadyan
	DEVICE_MODEL := AW1000
	BLOCKSIZE := 256k
	PAGESIZE := 4096
	DEVICE_DTS_CONFIG := config@hk09
	SOC := ipq8072
	DEVICE_PACKAGES := ipq-wifi-arcadyan_aw1000 kmod-spi-gpio \
		kmod-gpio-nxp-74hc164 kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += arcadyan_aw1000

define Device/buffalo_wxr-5950ax12
	$(call Device/FitImage)
	DEVICE_VENDOR := Buffalo
	DEVICE_MODEL := WXR-5950AX12
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk01
	SOC := ipq8074
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
	DEVICE_PACKAGES := ipq-wifi-buffalo_wxr-5950ax12
endef
TARGET_DEVICES += buffalo_wxr-5950ax12

define Device/cmcc_rm2-6
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := CMCC
	DEVICE_MODEL := RM2-6
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac02
	SOC := ipq8070
	IMAGE/factory.ubi := append-ubi | qsdk-ipq-factory-nand
	DEVICE_PACKAGES := ipq-wifi-cmcc_rm2-6 kmod-hwmon-gpiofan
endef
TARGET_DEVICES += cmcc_rm2-6

define Device/compex_wpq873
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Compex
	DEVICE_MODEL := WPQ873
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk09.wpq873
	SOC := ipq8072
	DEVICE_PACKAGES := ipq-wifi-compex_wpq873
	IMAGE/factory.ubi := append-ubi | qsdk-ipq-factory-nand
endef
TARGET_DEVICES += compex_wpq873

define Device/dynalink_dl-wrx36
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Dynalink
	DEVICE_MODEL := DL-WRX36
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@rt5010w-d350-rev0
	SOC := ipq8072
	DEVICE_PACKAGES := ipq-wifi-dynalink_dl-wrx36
endef
TARGET_DEVICES += dynalink_dl-wrx36

define Device/edgecore_eap102
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Edgecore
	DEVICE_MODEL := EAP102
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac02
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-edgecore_eap102
	IMAGE/factory.ubi := append-ubi | qsdk-ipq-factory-nand
endef
TARGET_DEVICES += edgecore_eap102

define Device/edimax_cax1800
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Edimax
	DEVICE_MODEL := CAX1800
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac03
	SOC := ipq8070
	DEVICE_PACKAGES := ipq-wifi-edimax_cax1800
endef
TARGET_DEVICES += edimax_cax1800

define Device/netgear_rax120v2
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := RAX120v2
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk01
	SOC := ipq8074
	KERNEL_SIZE := 29696k
	NETGEAR_BOARD_ID := RAX120
	NETGEAR_HW_ID := 29765589+0+512+1024+4x4+8x8
	DEVICE_PACKAGES := ipq-wifi-netgear_rax120v2 kmod-spi-gpio \
		kmod-spi-bitbang kmod-gpio-nxp-74hc164 kmod-hwmon-g761
	IMAGES = web-ui-factory.img sysupgrade.bin
	IMAGE/web-ui-factory.img := append-image initramfs-uImage.itb | \
		pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakehdr filesystem | \
		netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | \
		append-uImage-fakehdr filesystem | sysupgrade-tar kernel=$$$$@ | \
		append-metadata
endef
TARGET_DEVICES += netgear_rax120v2

define Device/netgear_wax218
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WAX218
	DEVICE_DTS_CONFIG := config@hk07
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq8072
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	ARTIFACTS := web-ui-factory.fit
	ARTIFACT/web-ui-factory.fit := append-image initramfs-uImage.itb | \
		ubinize-kernel | qsdk-ipq-factory-nand
endif
	DEVICE_PACKAGES := kmod-spi-gpio kmod-spi-bitbang kmod-gpio-nxp-74hc164 \
		ipq-wifi-netgear_wax218
endef
TARGET_DEVICES += netgear_wax218

define Device/netgear_wax620
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WAX620
	DEVICE_DTS_CONFIG := config@hk07
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq8072
	DEVICE_PACKAGES := kmod-spi-gpio kmod-gpio-nxp-74hc164 \
		ipq-wifi-netgear_wax620
endef
TARGET_DEVICES += netgear_wax620

define Device/netgear_wax630
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WAX630
	DEVICE_DTS_CONFIG := config@hk01
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq8074
	IMAGES := ui-factory.tar factory.ubi sysupgrade.bin
	IMAGE/ui-factory.tar := append-ubi | wax6xx-netgear-tar
	DEVICE_PACKAGES := kmod-spi-gpio ipq-wifi-netgear_wax630
endef
TARGET_DEVICES += netgear_wax630

define Device/prpl_haze
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := prpl Foundation
	DEVICE_MODEL := Haze
	DEVICE_DTS_CONFIG := config@hk09
	SOC := ipq8072
	DEVICE_PACKAGES := ath11k-firmware-qcn9074 ipq-wifi-prpl_haze kmod-ath11k-pci \
		mkf2fs f2fsck kmod-fs-f2fs
endef
TARGET_DEVICES += prpl_haze

define Device/qnap_301w
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := QNAP
	DEVICE_MODEL := 301w
	DEVICE_DTS_CONFIG := config@hk01
	KERNEL_SIZE := 16384k
	SOC := ipq8072
	DEVICE_PACKAGES := ipq-wifi-qnap_301w kmod-phy-aquantia
endef
TARGET_DEVICES += qnap_301w

define Device/redmi_ax6
	$(call Device/xiaomi_ax3600)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX6
	DEVICE_PACKAGES := ipq-wifi-redmi_ax6
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
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax3600
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
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax9000 kmod-ath11k-pci ath11k-firmware-qcn9074
endef
TARGET_DEVICES += xiaomi_ax9000

define Device/zbtlink_zbt-z800ax
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Zbtlink
	DEVICE_MODEL := ZBT-Z800AX
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@hk09
	SOC := ipq8074
	DEVICE_PACKAGES := ipq-wifi-zbtlink_zbt-z800ax
	IMAGES += factory.bin
	IMAGE/factory.bin := append-ubi | qsdk-ipq-factory-nand
endef
TARGET_DEVICES += zbtlink_zbt-z800ax

define Device/zte_mf269
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := ZTE
	DEVICE_MODEL := MF269
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ac04
	SOC := ipq8071
	DEVICE_PACKAGES := ipq-wifi-zte_mf269
endef
TARGET_DEVICES += zte_mf269

define Device/zyxel_nbg7815
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := ZYXEL
	DEVICE_MODEL := NBG7815
	DEVICE_DTS_CONFIG := config@nbg7815
	SOC := ipq8074
	DEVICE_PACKAGES := ipq-wifi-zyxel_nbg7815 kmod-ath11k-pci \
		kmod-bluetooth kmod-hwmon-tmp103
endef
TARGET_DEVICES += zyxel_nbg7815
