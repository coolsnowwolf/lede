DEVICE_VARS += UBNT_BOARD UBNT_CHIP UBNT_TYPE

# mkubntimage is using the kernel image direct
# routerboard creates partitions out of the ubnt header
define Build/mkubntimage
	-$(STAGING_DIR_HOST)/bin/mkfwimage \
		-B $(UBNT_BOARD) -v $(UBNT_TYPE).$(UBNT_CHIP).v6.0.0-$(VERSION_DIST)-$(REVISION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@
endef

# all UBNT XM device expect the kernel image to have 1024k while flash, when
# booting the image, the size doesn't matter.
define Build/mkubntimage-split
	-[ -f $@ ] && ( \
	dd if=$@ of=$@.old1 bs=1024k count=1; \
	dd if=$@ of=$@.old2 bs=1024k skip=1; \
	$(STAGING_DIR_HOST)/bin/mkfwimage \
		-B $(UBNT_BOARD) -v $(UBNT_TYPE).$(UBNT_CHIP).v6.0.0-$(VERSION_DIST)-$(REVISION) \
		-k $@.old1 \
		-r $@.old2 \
		-o $@; \
	rm $@.old1 $@.old2 )
endef

# UBNT_BOARD e.g. one of (XS2, XS5, RS, XM)
# UBNT_TYPE e.g. one of (BZ, XM, XW)
# UBNT_CHIP e.g. one of (ar7240, ar933x, ar934x)
define Device/ubnt
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  IMAGE_SIZE := 7552k
  UBNT_BOARD := XM
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | mkubntimage-split
endef

define Device/ubnt-xm
  $(Device/ubnt)
  DEVICE_PACKAGES += kmod-usb-ohci
  UBNT_TYPE := XM
  UBNT_CHIP := ar7240
  ATH_SOC := ar7241
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma | uImage lzma
endef

define Device/ubnt-bz
  $(Device/ubnt)
  UBNT_TYPE := BZ
  UBNT_CHIP := ar7240
  ATH_SOC := ar7241
endef

define Device/ubnt_bullet-m
  $(Device/ubnt-xm)
  DEVICE_TITLE := Ubiquiti Bullet-M
  SUPPORTED_DEVICES += bullet-m
endef
TARGET_DEVICES += ubnt_bullet-m

define Device/ubnt_rocket-m
  $(Device/ubnt-xm)
  DEVICE_TITLE := Ubiquiti Rocket-M
  SUPPORTED_DEVICES += rocket-m
endef
TARGET_DEVICES += ubnt_rocket-m

define Device/ubnt_nano-m
  $(Device/ubnt-xm)
  DEVICE_TITLE := Ubiquiti Nano-M
  SUPPORTED_DEVICES += nano-m
endef
TARGET_DEVICES += ubnt_nano-m

define Device/ubnt_unifi
  $(Device/ubnt-bz)
  DEVICE_TITLE := Ubiquiti UniFi
  SUPPORTED_DEVICES += unifi
endef
TARGET_DEVICES += ubnt_unifi

define Device/ubnt_unifiac
  ATH_SOC := qca9563
  IMAGE_SIZE := 7744k
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
endef


define Device/ubnt_unifiac-lite
  $(Device/ubnt_unifiac)
  DEVICE_TITLE := Ubiquiti UniFi AC-Lite
  SUPPORTED_DEVICES += ubnt-unifiac-lite
endef
TARGET_DEVICES += ubnt_unifiac-lite

define Device/ubnt_unifiac-mesh
  $(Device/ubnt_unifiac)
  DEVICE_TITLE := Ubiquiti UniFi AC-Mesh
  SUPPORTED_DEVICES += ubnt-unifiac-mesh
endef
TARGET_DEVICES += ubnt_unifiac-mesh

define Device/ubnt_unifiac-mesh-pro
  $(Device/ubnt_unifiac)
  DEVICE_TITLE := Ubiquiti UniFi AC-Mesh Pro
  SUPPORTED_DEVICES += ubnt-unifiac-mesh-pro
endef
TARGET_DEVICES += ubnt_unifiac-mesh-pro

define Device/ubnt_unifiac-pro
  $(Device/ubnt_unifiac)
  DEVICE_TITLE := Ubiquiti UniFi AC-Pro
  DEVICE_PACKAGES += kmod-usb-core kmod-usb2
  SUPPORTED_DEVICES += ubnt-unifiac-pro
endef
TARGET_DEVICES += ubnt_unifiac-pro

define Device/ubnt_routerstation_common
  DEVICE_PACKAGES := -kmod-ath9k -wpad-mini -uboot-envtools kmod-usb-ohci kmod-usb2 fconfig
  ATH_SOC := ar7161
  IMAGE_SIZE := 16128k
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/factory.bin := append-rootfs | pad-rootfs | mkubntimage | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | combined-image | check-size $$$$(IMAGE_SIZE) | append-metadata
  KERNEL := kernel-bin | append-dtb | lzma | pad-to $$(BLOCKSIZE)
  KERNEL_INITRAMFS := kernel-bin | append-dtb
endef

define Device/ubnt_routerstation
  $(Device/ubnt_routerstation_common)
  DEVICE_TITLE := Ubiquiti RouterStation
  UBNT_BOARD := RS
  UBNT_TYPE := RSx
  UBNT_CHIP := ar7100
  DEVICE_PACKAGES += -swconfig
endef
TARGET_DEVICES += ubnt_routerstation

define Device/ubnt_routerstation-pro
  $(Device/ubnt_routerstation_common)
  DEVICE_TITLE := Ubiquiti RouterStation Pro
  UBNT_BOARD := RSPRO
  UBNT_TYPE := RSPRO
  UBNT_CHIP := ar7100pro
endef
TARGET_DEVICES += ubnt_routerstation-pro
