include ./common-mikrotik.mk

define Device/mikrotik_routerboard-493g
  $(Device/mikrotik)
  SOC := ar7161
  DEVICE_MODEL := RouterBOARD 493G
  IMAGE/sysupgrade.bin = append-kernel | kernel2minor -s 2048 -e -c | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES += kmod-usb-ohci kmod-usb2 nand-utils
  SUPPORTED_DEVICES += rb-493g
endef
TARGET_DEVICES += mikrotik_routerboard-493g

define Device/mikrotik_routerboard-922uags-5hpacd
  $(Device/mikrotik)
  SOC := qca9558
  DEVICE_MODEL := RouterBOARD 922UAGS-5HPacD
  IMAGE/sysupgrade.bin = append-kernel | kernel2minor -s 2048 -e -c | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES += kmod-ath10k-ct ath10k-firmware-qca988x-ct \
	kmod-usb2 nand-utils
  SUPPORTED_DEVICES += rb-922uags-5hpacd
endef
TARGET_DEVICES += mikrotik_routerboard-922uags-5hpacd

define Device/mikrotik_routerboard-wap-g-5hact2hnd
  $(Device/mikrotik)
  SOC := qca9556
  DEVICE_MODEL := RouterBOARD wAP G-5HacT2HnD (wAP AC)
  IMAGE_SIZE := 16256k
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	append-metadata | check-size
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += rb-wapg-5hact2hnd
endef
TARGET_DEVICES += mikrotik_routerboard-wap-g-5hact2hnd
