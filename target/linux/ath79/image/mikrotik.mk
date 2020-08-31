include ./common-mikrotik.mk

define Device/mikrotik_routerboard-493g
  $(Device/mikrotik_nand)
  SOC := ar7161
  DEVICE_MODEL := RouterBOARD 493G
  DEVICE_PACKAGES += kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += rb-493g
endef
TARGET_DEVICES += mikrotik_routerboard-493g

define Device/mikrotik_routerboard-921gs-5hpacd-15s
  $(Device/mikrotik_nand)
  SOC := qca9558
  DEVICE_MODEL := RouterBOARD 921GS-5HPacD-15s (mANTBox 15s)
  DEVICE_PACKAGES += kmod-ath10k-ct ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += rb-921gs-5hpacd-r2
endef
TARGET_DEVICES += mikrotik_routerboard-921gs-5hpacd-15s

define Device/mikrotik_routerboard-922uags-5hpacd
  $(Device/mikrotik_nand)
  SOC := qca9558
  DEVICE_MODEL := RouterBOARD 922UAGS-5HPacD
  DEVICE_PACKAGES += kmod-ath10k-ct ath10k-firmware-qca988x-ct kmod-usb2
  SUPPORTED_DEVICES += rb-922uags-5hpacd
endef
TARGET_DEVICES += mikrotik_routerboard-922uags-5hpacd

define Device/mikrotik_routerboard-lhg-2nd
  $(Device/mikrotik_nor)
  SOC := qca9533
  DEVICE_MODEL := RouterBOARD LHG 2nD (LHG 2)
  IMAGE_SIZE := 16256k
endef
TARGET_DEVICES += mikrotik_routerboard-lhg-2nd

define Device/mikrotik_routerboard-sxt-5nd-r2
  $(Device/mikrotik_nand)
  SOC := ar9344
  DEVICE_MODEL := RouterBOARD SXT 5nD r2 (SXT Lite5)
  DEVICE_PACKAGES += rssileds kmod-gpio-beeper
  SUPPORTED_DEVICES += rb-sxt5n
endef
TARGET_DEVICES += mikrotik_routerboard-sxt-5nd-r2

define Device/mikrotik_routerboard-wap-g-5hact2hnd
  $(Device/mikrotik_nor)
  SOC := qca9556
  DEVICE_MODEL := RouterBOARD wAP G-5HacT2HnD (wAP AC)
  IMAGE_SIZE := 16256k
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += rb-wapg-5hact2hnd
endef
TARGET_DEVICES += mikrotik_routerboard-wap-g-5hact2hnd
