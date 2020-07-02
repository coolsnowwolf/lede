define Device/arcadyan_arv4510pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4510PW
  DEVICE_ALT0_VENDOR := Wippies
  DEVICE_ALT0_MODEL := BeWan iBox v1.0
  IMAGE_SIZE := 15616k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-ledtrig-usbport kmod-usb2-pci kmod-usb-uhci \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci kmod-ath5k wpad-basic
  SUPPORTED_DEVICES += ARV4510PW
endef
TARGET_DEVICES += arcadyan_arv4510pw

define Device/arcadyan_arv4519pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4519PW
  DEVICE_ALT0_VENDOR := Vodafone
  DEVICE_ALT0_MODEL := NetFasteR IAD 2
  DEVICE_ALT1_VENDOR := Pirelli
  DEVICE_ALT1_MODEL := P.RG A4201G
  IMAGE_SIZE := 3776k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += ARV4519PW
endef
TARGET_DEVICES += arcadyan_arv4519pw

define Device/arcadyan_arv7506pw11
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7506PW11
  DEVICE_ALT0_VENDOR := Alice/O2
  DEVICE_ALT0_MODEL := IAD 4421
  IMAGE_SIZE := 7808k
  SOC := danube
  DEVICE_PACKAGES := kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-basic
  SUPPORTED_DEVICES += ARV7506PW11
endef
TARGET_DEVICES += arcadyan_arv7506pw11

define Device/arcadyan_arv7510pw22
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7510PW22
  DEVICE_ALT0_VENDOR := Astoria Networks
  DEVICE_ALT0_MODEL := ARV7510PW22
  IMAGE_SIZE := 31232k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-basic \
	kmod-usb-uhci kmod-usb2 kmod-usb2-pci
  SUPPORTED_DEVICES += ARV7510PW22
endef
TARGET_DEVICES += arcadyan_arv7510pw22

define Device/arcadyan_arv7518pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7518PW
  DEVICE_ALT0_VENDOR := Astoria Networks
  DEVICE_ALT0_MODEL := ARV7518PW
  IMAGE_SIZE := 7872k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic
  SUPPORTED_DEVICES += ARV7518PW
endef
TARGET_DEVICES += arcadyan_arv7518pw

define Device/arcadyan_arv7519pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7519PW
  DEVICE_ALT0_VENDOR := Astoria Networks
  DEVICE_ALT0_MODEL := ARV7519PW
  IMAGE_SIZE := 15488k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-basic
  SUPPORTED_DEVICES += ARV7519PW
endef
TARGET_DEVICES += arcadyan_arv7519pw

define Device/arcadyan_arv7525pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7525PW
  DEVICE_ALT0_VENDOR := Telekom
  DEVICE_ALT0_MODEL := Speedport W303V
  DEVICE_ALT0_VARIANT := Typ A
  IMAGE_SIZE := 3776k
  SOC := danube
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
  SUPPORTED_DEVICES += ARV4510PW
endef
TARGET_DEVICES += arcadyan_arv7525pw

define Device/arcadyan_arv752dpw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV752DPW
  DEVICE_ALT0_VENDOR := Vodafone
  DEVICE_ALT0_MODEL := Easybox 802
  IMAGE_SIZE := 7872k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-basic
  SUPPORTED_DEVICES += ARV752DPW
endef
TARGET_DEVICES += arcadyan_arv752dpw

define Device/arcadyan_arv752dpw22
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV752DPW22
  DEVICE_ALT0_VENDOR := Vodafone
  DEVICE_ALT0_MODEL := Easybox 803
  IMAGE_SIZE := 7616k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb2-pci kmod-usb-uhci kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-basic
  SUPPORTED_DEVICES += ARV752DPW22
endef
TARGET_DEVICES += arcadyan_arv752dpw22

define Device/arcadyan_arv8539pw22
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV8539PW22
  DEVICE_ALT0_VENDOR := Telekom
  DEVICE_ALT0_MODEL := Speedport W504V Typ A
  IMAGE_SIZE := 7616k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic
  SUPPORTED_DEVICES += ARV8539PW22
endef
TARGET_DEVICES += arcadyan_arv8539pw22

define Device/audiocodes_mp-252
  DEVICE_VENDOR := AudioCodes
  DEVICE_MODEL := MediaPack MP-252
  IMAGE_SIZE := 14848k
  SOC := danube
  DEVICE_PACKAGES :=  kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-usb-ledtrig-usbport kmod-usb-dwc2 \
	kmod-rt2800-pci \
	ltq-adsl-app ppp-mod-pppoa \
	wpad-basic
  SUPPORTED_DEVICES += ACMP252
endef
TARGET_DEVICES += audiocodes_mp-252

define Device/bt_homehub-v2b
  $(Device/NAND)
  DEVICE_VENDOR := British Telecom
  DEVICE_MODEL := Home Hub 2
  DEVICE_VARIANT := Type B
  BOARD_NAME := BTHOMEHUBV2B
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-deu-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic
  SUPPORTED_DEVICES += BTHOMEHUBV2B
endef
TARGET_DEVICES += bt_homehub-v2b

define Device/lantiq_easy50712
  DEVICE_VENDOR := Lantiq
  DEVICE_MODEL := Danube (EASY50712)
  SOC := danube
  IMAGE_SIZE := 3776k
endef
TARGET_DEVICES += lantiq_easy50712

define Device/siemens_gigaset-sx76x
  DEVICE_VENDOR := Siemens
  DEVICE_MODEL := Gigaset sx76x
  IMAGE_SIZE := 7680k
  SOC := danube
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ath5k wpad-basic
  SUPPORTED_DEVICES += GIGASX76X
endef
TARGET_DEVICES += siemens_gigaset-sx76x
