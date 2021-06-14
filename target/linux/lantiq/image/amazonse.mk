define Device/allnet_all0333cj
  DEVICE_VENDOR := Allnet
  DEVICE_MODEL := ALL0333CJ
  IMAGE_SIZE := 3700k
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
endef
TARGET_DEVICES += allnet_all0333cj

define Device/netgear_dgn1000b
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DGN1000B
  IMAGE_SIZE := 6000k
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
  SUPPORTED_DEVICES += DGN1000B
endef
TARGET_DEVICES += netgear_dgn1000b
