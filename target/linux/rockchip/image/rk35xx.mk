
define Device/rk3528
  SOC := rk3528
  DEVICE_DTS_DIR := ../dts/rk3528
  DEVICE_DTS = $$(SOC)-$$(lastword $$(subst _, ,$$(DEVICE_NAME)))
  UBOOT_DEVICE_NAME := rk3528-evb
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
endef

define Device/armsom_sige1
$(call Device/rk3528)
  DEVICE_VENDOR := ArmSoM
  DEVICE_MODEL := Sige1
  SUPPORTED_DEVICES += armsom,sige1 armsom,sige1-v1 armsom,sige1-v1.0
  DEVICE_PACKAGES := kmod-r8125 kmod-thermal kmod-brcmfmac brcmfmac-firmware-43752-sdio brcmfmac-nvram-43752-sdio
endef
TARGET_DEVICES += armsom_sige1

define Device/radxa_e20c
$(call Device/rk3528)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E20C
  SUPPORTED_DEVICES += radxa,e20c
  DEVICE_DTS := rk3528-e20c
  DEVICE_PACKAGES := kmod-r8168 kmod-r8125 kmod-thermal
endef
TARGET_DEVICES += radxa_e20c
