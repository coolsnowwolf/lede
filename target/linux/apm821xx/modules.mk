define KernelPackage/ata-dwc
  TITLE:=DesignWare Cores SATA support
  KCONFIG:=CONFIG_SATA_DWC
  FILES:=$(LINUX_DIR)/drivers/ata/sata_dwc_460ex.ko
  AUTOLOAD:=$(call AutoLoad,40,sata_dwc_460ex,1)
  $(call AddDepends/ata,@TARGET_apm821xx)
endef

define KernelPackage/ata-dwc/description
 Platform support for the on-chip SATA controller.
endef

$(eval $(call KernelPackage,ata-dwc))

define KernelPackage/hw-crypto-4xx
  TITLE:=Driver AMCC PPC4xx crypto accelerator
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_HW_RANDOM=y \
	CONFIG_CRYPTO_DEV_PPC4XX \
	CONFIG_HW_RANDOM_PPC4XX=y
  DEPENDS:=@TARGET_apm821xx +kmod-random-core +kmod-crypto-manager \
	   +kmod-crypto-ccm +kmod-crypto-gcm \
	   +kmod-crypto-sha1 +kmod-crypto-sha256 +kmod-crypto-sha512
  FILES:=$(LINUX_DIR)/drivers/crypto/amcc/crypto4xx.ko
  AUTOLOAD:=$(call AutoLoad,09,crypto4xx,1)
  $(call AddDepends/crypto)
endef

define KernelPackage/hw-crypto-4xx/description
  Platform support for the on-chip crypto acceleration.
endef

$(eval $(call KernelPackage,hw-crypto-4xx))
