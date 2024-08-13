
define Device/rk3528
  SOC := rk3528
  DEVICE_DTS_DIR := ../dts/rk3528
  DEVICE_DTS = $$(SOC)-$$(lastword $$(subst _, ,$$(DEVICE_NAME)))
  UBOOT_DEVICE_NAME := rk3528-evb
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
endef

define Device/rk3588
  SOC := rk3588
  DEVICE_DTS_DIR := ../dts/rk3588
  DEVICE_DTS = $$(SOC)-$$(lastword $$(subst _, ,$$(DEVICE_NAME)))
  UBOOT_DEVICE_NAME := rk3588-evb
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3588 | pine64-img | gzip | append-metadata
endef

define Device/armsom_sige1
$(call Device/rk3528)
  DEVICE_VENDOR := ArmSoM
  DEVICE_MODEL := Sige1
  SUPPORTED_DEVICES += armsom,sige1 armsom,sige1-v1 armsom,sige1-v1.0
  DEVICE_PACKAGES := kmod-r8125 kmod-thermal kmod-brcmfmac brcmfmac-firmware-43752-sdio brcmfmac-nvram-43752-sdio
endef
TARGET_DEVICES += armsom_sige1

define Device/armsom_sige7
$(call Device/rk3588)
  DEVICE_VENDOR := ArmSoM
  DEVICE_MODEL := Sige7
  DEVICE_DTS := rk3588-sige7-v1-1
  SUPPORTED_DEVICES += armsom,sige7 armsom,sige7-v1 armsom,sige7-v1.0
  DEVICE_PACKAGES := kmod-r8125 kmod-nvme kmod-hwmon-pwmfan kmod-thermal \
	kmod-brcmfmac brcmfmac-firmware-43752-pcie brcmfmac-nvram-43752-pcie
endef
TARGET_DEVICES += armsom_sige7

define Device/mangopi_m28k
$(call Device/rk3528)
  DEVICE_VENDOR := MangoPi
  DEVICE_MODEL := M28K / M28K Pro
  SUPPORTED_DEVICES := mangopi,m28k widora,mangopi-m28k
  DEVICE_PACKAGES := kmod-r8168 kmod-r8125 kmod-thermal
endef
TARGET_DEVICES += mangopi_m28k

define Device/radxa_e20c
$(call Device/rk3528)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E20C
  SUPPORTED_DEVICES += radxa,e20c
  DEVICE_DTS := rk3528-e20c
  DEVICE_PACKAGES := kmod-r8168 kmod-r8125 kmod-thermal
endef
TARGET_DEVICES += radxa_e20c
