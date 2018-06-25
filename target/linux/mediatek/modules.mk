define KernelPackage/ata-ahci-mtk
  TITLE:=Mediatek AHCI Serial ATA support
  KCONFIG:=CONFIG_AHCI_MTK
  FILES:= \
	$(LINUX_DIR)/drivers/ata/ahci_mtk.ko \
	$(LINUX_DIR)/drivers/ata/libahci_platform.ko
  AUTOLOAD:=$(call AutoLoad,40,libahci libahci_platform ahci_mtk,1)
  $(call AddDepends/ata)
  DEPENDS+=@TARGET_mediatek_mt7622
endef

define KernelPackage/ata-ahci-mtk/description
 Mediatek AHCI Serial ATA host controllers
endef

$(eval $(call KernelPackage,ata-ahci-mtk))

define KernelPackage/sdhci-mtk
  SUBMENU:=Other modules
  TITLE:=Mediatek SDHCI driver
  DEPENDS:=@TARGET_mediatek_mt7622 +kmod-sdhci
  KCONFIG:=CONFIG_MMC_MTK 
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/mtk-sd.ko
  AUTOLOAD:=$(call AutoProbe,mtk-sd,1)
endef

$(eval $(call KernelPackage,sdhci-mtk))

define KernelPackage/crypto-hw-mtk
  TITLE:= MediaTek's Crypto Engine module
  DEPENDS:=@TARGET_mediatek
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_AES=y \
	CONFIG_CRYPTO_AEAD=y \
	CONFIG_CRYPTO_SHA1=y \
	CONFIG_CRYPTO_SHA256=y \
	CONFIG_CRYPTO_SHA512=y \
	CONFIG_CRYPTO_HMAC=y \
	CONFIG_CRYPTO_DEV_MEDIATEK
  FILES:=$(LINUX_DIR)/drivers/crypto/mediatek/mtk-crypto.ko
  AUTOLOAD:=$(call AutoLoad,90,mtk-crypto)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-hw-mtk/description
  MediaTek's EIP97 Cryptographic Engine driver.
endef

$(eval $(call KernelPackage,crypto-hw-mtk))
