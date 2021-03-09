define Device/arcadyan_arv4518pwr01
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4518PWR01
  IMAGE_SIZE := 3776k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-basic-wolfssl
  SUPPORTED_DEVICES += ARV4518PWR01
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv4518pwr01

define Device/arcadyan_arv4518pwr01a
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4518PWR01A
  IMAGE_SIZE := 3776k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-basic-wolfssl
  SUPPORTED_DEVICES += ARV4518PWR01A
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv4518pwr01a

define Device/arcadyan_arv4520pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4520PW
  DEVICE_ALT0_VENDOR := Vodafone
  DEVICE_ALT0_MODEL := Easybox 800
  DEVICE_ALT1_VENDOR := Airties
  DEVICE_ALT1_MODEL := WAV-281
  IMAGE_SIZE := 3648k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt61-pci wpad-basic-wolfssl
  SUPPORTED_DEVICES += ARV4520PW
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv4520pw

define Device/arcadyan_arv4525pw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV4525PW
  DEVICE_ALT0_VENDOR := Telekom
  DEVICE_ALT0_MODEL := Speedport W502V
  DEVICE_ALT0_VARIANT := Typ A
  IMAGE_SIZE := 3776k
  DEVICE_PACKAGES := kmod-ath5k wpad-basic-wolfssl \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
  SUPPORTED_DEVICES += ARV4525PW
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv4525pw

define Device/arcadyan_arv452cqw
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV452CQW
  DEVICE_ALT0_VENDOR := Vodafone
  DEVICE_ALT0_MODEL := Easybox 801
  IMAGE_SIZE := 3776k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath5k wpad-basic-wolfssl \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += ARV452CQW
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv452cqw
