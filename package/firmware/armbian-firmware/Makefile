include $(TOPDIR)/rules.mk

PKG_NAME:=armbian-firmware
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2024-06-26
PKG_SOURCE_URL:=https://github.com/armbian/firmware.git
PKG_SOURCE_VERSION:=7f2e18ecac75d50acb843395ad07a0f2b9a12a50
PKG_MIRROR_HASH:=9967883add1587fb178e3b1a9aecec8e03b67f3554af63b294b771f67b88c934

include $(INCLUDE_DIR)/package.mk

RSTRIP:=:
STRIP:=:

define Package/armbian-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://github.com/armbian/firmware
  TITLE:=$(1)
  DEPENDS:=$(2)
endef

define Build/Compile
endef

Package/brcmfmac-firmware-4356-sdio = $(call Package/armbian-firmware-default,Broadcom BCM4356 FullMac SDIO firmware)
define Package/brcmfmac-firmware-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/BCM4356A2.hcd \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4356-sdio))

Package/brcmfmac-firmware-43456-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43456 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43456-sdio))

Package/brcmfmac-firmware-43752-pcie = $(call Package/armbian-firmware-default,Broadcom BCM43752 FullMac PCIe firmware)
define Package/brcmfmac-firmware-43752-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/BCM4362A2.hcd \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-pcie.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-pcie.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43752-pcie))

Package/brcmfmac-firmware-43752-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43752 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43752-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/BCM4362A2.hcd \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43752-sdio))

Package/brcmfmac-nvram-4356-sdio = $(call Package/armbian-firmware-default,Broadcom BCM4356 SDIO NVRAM firmware)
define Package/brcmfmac-nvram-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac4356-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.friendlyarm,nanopc-t4.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-4356-sdio))

Package/brcmfmac-nvram-43456-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43456 SDIO NVRAM firmware)
define Package/brcmfmac-nvram-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac43456-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.radxa,rockpi4a.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43456-sdio))

Package/brcmfmac-nvram-43752-pcie = $(call Package/armbian-firmware-default,Broadcom BCM43752 PCIe NVRAM firmware)
define Package/brcmfmac-nvram-43752-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-pcie.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac43752-pcie.txt \
		$(1)/lib/firmware/brcm/brcmfmac43752-pcie.armsom,sige7.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43752-pcie))

Package/brcmfmac-nvram-43752-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43752 SDIO NVRAM firmware)
define Package/brcmfmac-nvram-43752-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac43752-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43752-sdio.armsom,sige1-v1.txt
	$(LN) \
		brcmfmac43752-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43752-sdio.firefly,rk3568-roc-pc.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43752-sdio))
