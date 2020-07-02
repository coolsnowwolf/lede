DEVICE_VARS += UBNT_BOARD UBNT_CHIP UBNT_TYPE UBNT_VERSION UBNT_REVISION

# On M (XW) devices the U-Boot as of version 1.1.4-s1039 doesn't like
# VERSION_DIST being on the place of major(?) version number, so we need to
# use some number.
UBNT_REVISION := $(VERSION_DIST)-$(REVISION)

# mkubntimage is using the kernel image direct
# routerboard creates partitions out of the ubnt header
define Build/mkubntimage
	-$(STAGING_DIR_HOST)/bin/mkfwimage -B $(UBNT_BOARD) \
		-v $(UBNT_TYPE).$(UBNT_CHIP).v6.0.0-$(VERSION_DIST)-$(REVISION) \
		-k $(IMAGE_KERNEL) -r $@ -o $@
endef

# all UBNT XM/WA devices expect the kernel image to have 1024k while flash, when
# booting the image, the size doesn't matter.
define Build/mkubntimage-split
	-[ -f $@ ] && ( \
	dd if=$@ of=$@.old1 bs=1024k count=1; \
	dd if=$@ of=$@.old2 bs=1024k skip=1; \
	$(STAGING_DIR_HOST)/bin/mkfwimage -B $(UBNT_BOARD) \
		-v $(UBNT_TYPE).$(UBNT_CHIP).v$(UBNT_VERSION)-$(UBNT_REVISION) \
		-k $@.old1 -r $@.old2 -o $@; \
	rm $@.old1 $@.old2 )
endef

# UBNT_BOARD e.g. one of (XS2, XS5, RS, XM)
# UBNT_TYPE e.g. one of (BZ, XM, XW)
# UBNT_CHIP e.g. one of (ar7240, ar933x, ar934x)
# UBNT_VERSION e.g. one of (6.0.0, 8.5.3)
define Device/ubnt
  DEVICE_VENDOR := Ubiquiti
  DEVICE_PACKAGES := kmod-usb2
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size | mkubntimage-split
endef

define Device/ubnt-bz
  $(Device/ubnt)
  SOC := ar7241
  IMAGE_SIZE := 7448k
  UBNT_BOARD := XM
  UBNT_CHIP := ar7240
  UBNT_TYPE := BZ
  UBNT_VERSION := 6.0.0
endef

define Device/ubnt-sw
  $(Device/ubnt)
  SOC := ar7242
  DEVICE_PACKAGES += kmod-usb-ohci
  IMAGE_SIZE := 7552k
  UBNT_BOARD := SW
  UBNT_CHIP := ar7240
  UBNT_TYPE := SW
  UBNT_VERSION := 1.4.1
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma | uImage lzma
endef

define Device/ubnt-wa
  $(Device/ubnt)
  SOC := ar9342
  IMAGE_SIZE := 15744k
  UBNT_BOARD := WA
  UBNT_CHIP := ar934x
  UBNT_TYPE := WA
  UBNT_VERSION := 8.5.3
endef

define Device/ubnt-xc
  $(Device/ubnt)
  IMAGE_SIZE := 15744k
  UBNT_BOARD := XC
  UBNT_CHIP := qca955x
  UBNT_TYPE := XC
  UBNT_VERSION := 8.5.3
endef

define Device/ubnt-xm
  $(Device/ubnt)
  DEVICE_VARIANT := XM
  DEVICE_PACKAGES += kmod-usb-ohci
  IMAGE_SIZE := 7448k
  UBNT_BOARD := XM
  UBNT_CHIP := ar7240
  UBNT_TYPE := XM
  UBNT_VERSION := 6.0.0
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma | uImage lzma
endef

define Device/ubnt-xw
  $(Device/ubnt)
  SOC := ar9342
  DEVICE_VARIANT := XW
  IMAGE_SIZE := 7552k
  UBNT_BOARD := XM
  UBNT_CHIP := ar934x
  UBNT_REVISION := 42.$(UBNT_REVISION)
  UBNT_TYPE := XW
  UBNT_VERSION := 6.0.4
endef

define Device/ubnt_acb-isp
  $(Device/ubnt)
  SOC := qca9533
  DEVICE_MODEL := airCube ISP
  IMAGE_SIZE := 15744k
  UBNT_BOARD := ACB-ISP
  UBNT_CHIP := qca9533
  UBNT_TYPE := ACB
  UBNT_VERSION := 2.5.0
endef
TARGET_DEVICES += ubnt_acb-isp

define Device/ubnt_airrouter
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := AirRouter
  SUPPORTED_DEVICES += airrouter
endef
TARGET_DEVICES += ubnt_airrouter

define Device/ubnt_bullet-m-ar7240
  $(Device/ubnt-xm)
  SOC := ar7240
  DEVICE_MODEL := Bullet-M
  DEVICE_VARIANT := XM (AR7240)
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m
endef
TARGET_DEVICES += ubnt_bullet-m-ar7240

define Device/ubnt_bullet-m-ar7241
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := Bullet-M
  DEVICE_VARIANT := XM (AR7241)
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m ubnt,bullet-m
endef
TARGET_DEVICES += ubnt_bullet-m-ar7241

define Device/ubnt_bullet-m-xw
  $(Device/ubnt-xw)
  DEVICE_MODEL := Bullet-M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m-xw
endef
TARGET_DEVICES += ubnt_bullet-m-xw

define Device/ubnt_edgeswitch-5xp
  $(Device/ubnt-sw)
  DEVICE_MODEL := EdgeSwitch 5XP
endef
TARGET_DEVICES += ubnt_edgeswitch-5xp

define Device/ubnt_edgeswitch-8xp
  $(Device/ubnt-sw)
  DEVICE_MODEL := EdgeSwitch 8XP
  DEVICE_PACKAGES += kmod-switch-bcm53xx-mdio
endef
TARGET_DEVICES += ubnt_edgeswitch-8xp

define Device/ubnt_lap-120
  $(Device/ubnt-wa)
  DEVICE_MODEL := LiteAP ac
  DEVICE_VARIANT := LAP-120
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += ubnt_lap-120

define Device/ubnt_litebeam-ac-gen2
  $(Device/ubnt-wa)
  DEVICE_MODEL := LiteBeam AC
  DEVICE_VARIANT := Gen2
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += ubnt_litebeam-ac-gen2

define Device/ubnt_nanobeam-ac
  $(Device/ubnt-wa)
  DEVICE_MODEL := NanoBeam AC
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct rssileds
endef
TARGET_DEVICES += ubnt_nanobeam-ac

define Device/ubnt_nanobridge-m
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := NanoBridge M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m
endef
TARGET_DEVICES += ubnt_nanobridge-m

define Device/ubnt_nanostation-ac
  $(Device/ubnt-wa)
  DEVICE_MODEL := Nanostation AC
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct rssileds
endef
TARGET_DEVICES += ubnt_nanostation-ac

define Device/ubnt_nanostation-ac-loco
  $(Device/ubnt-wa)
  DEVICE_MODEL := Nanostation AC loco
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += ubnt_nanostation-ac-loco

define Device/ubnt_nanostation-loco-m
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := Nanostation Loco M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m
endef
TARGET_DEVICES += ubnt_nanostation-loco-m

define Device/ubnt_nanostation-loco-m-xw
  $(Device/ubnt-xw)
  DEVICE_MODEL := Nanostation Loco M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += loco-m-xw
endef
TARGET_DEVICES += ubnt_nanostation-loco-m-xw

define Device/ubnt_nanostation-m
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := Nanostation M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += nanostation-m
endef
TARGET_DEVICES += ubnt_nanostation-m

define Device/ubnt_nanostation-m-xw
  $(Device/ubnt-xw)
  DEVICE_MODEL := Nanostation M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += nanostation-m-xw
endef
TARGET_DEVICES += ubnt_nanostation-m-xw

define Device/ubnt_picostation-m
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := Picostation M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += bullet-m
endef
TARGET_DEVICES += ubnt_picostation-m

define Device/ubnt_powerbeam-5ac-500
  $(Device/ubnt-xc)
  SOC := qca9558
  DEVICE_MODEL := PowerBeam 5AC
  DEVICE_VARIANT := 500
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += ubnt_powerbeam-5ac-500

define Device/ubnt_powerbeam-5ac-gen2
  $(Device/ubnt-wa)
  DEVICE_MODEL := PowerBeam 5AC
  DEVICE_VARIANT := Gen2
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct rssileds
endef
TARGET_DEVICES += ubnt_powerbeam-5ac-gen2

define Device/ubnt_rocket-m
  $(Device/ubnt-xm)
  SOC := ar7241
  DEVICE_MODEL := Rocket-M
  DEVICE_PACKAGES += rssileds
  SUPPORTED_DEVICES += rocket-m
endef
TARGET_DEVICES += ubnt_rocket-m

define Device/ubnt_routerstation_common
  DEVICE_PACKAGES := -kmod-ath9k -wpad-mini -uboot-envtools kmod-usb-ohci \
	kmod-usb2 fconfig
  DEVICE_VENDOR := Ubiquiti
  SOC := ar7161
  IMAGE_SIZE := 16128k
  IMAGES := factory.bin
  IMAGE/factory.bin := append-rootfs | pad-rootfs | mkubntimage | \
	check-size
  KERNEL := kernel-bin | append-dtb | lzma | pad-to $$(BLOCKSIZE)
  KERNEL_INITRAMFS := kernel-bin | append-dtb
endef

define Device/ubnt_routerstation
  $(Device/ubnt_routerstation_common)
  DEVICE_MODEL := RouterStation
  UBNT_BOARD := RS
  UBNT_TYPE := RSx
  UBNT_CHIP := ar7100
  DEVICE_PACKAGES += -swconfig
  SUPPORTED_DEVICES += routerstation
endef
TARGET_DEVICES += ubnt_routerstation

define Device/ubnt_routerstation-pro
  $(Device/ubnt_routerstation_common)
  DEVICE_MODEL := RouterStation Pro
  UBNT_BOARD := RSPRO
  UBNT_TYPE := RSPRO
  UBNT_CHIP := ar7100pro
  SUPPORTED_DEVICES += routerstation-pro
endef
TARGET_DEVICES += ubnt_routerstation-pro

define Device/ubnt_unifi
  $(Device/ubnt-bz)
  DEVICE_MODEL := UniFi
  SUPPORTED_DEVICES += unifi
endef
TARGET_DEVICES += ubnt_unifi

define Device/ubnt_unifiac
  DEVICE_VENDOR := Ubiquiti
  SOC := qca9563
  IMAGE_SIZE := 7744k
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef

define Device/ubnt_unifiac-lite
  $(Device/ubnt_unifiac)
  DEVICE_MODEL := UniFi AC-Lite
  SUPPORTED_DEVICES += unifiac-lite
endef
TARGET_DEVICES += ubnt_unifiac-lite

define Device/ubnt_unifiac-lr
  $(Device/ubnt_unifiac)
  DEVICE_MODEL := UniFi AC-LR
  SUPPORTED_DEVICES += unifiac-lite ubnt,unifiac-lite
endef
TARGET_DEVICES += ubnt_unifiac-lr

define Device/ubnt_unifiac-mesh
  $(Device/ubnt_unifiac)
  DEVICE_MODEL := UniFi AC-Mesh
  SUPPORTED_DEVICES += unifiac-lite
endef
TARGET_DEVICES += ubnt_unifiac-mesh

define Device/ubnt_unifiac-mesh-pro
  $(Device/ubnt_unifiac)
  DEVICE_MODEL := UniFi AC-Mesh Pro
  SUPPORTED_DEVICES += unifiac-pro
endef
TARGET_DEVICES += ubnt_unifiac-mesh-pro

define Device/ubnt_unifiac-pro
  $(Device/ubnt_unifiac)
  DEVICE_MODEL := UniFi AC-Pro
  DEVICE_PACKAGES += kmod-usb2
  SUPPORTED_DEVICES += unifiac-pro
endef
TARGET_DEVICES += ubnt_unifiac-pro
