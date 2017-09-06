Package/brcmfmac-firmware-43602a1-pcie = $(call Package/firmware-default,Broadcom 43602a1 FullMAC PCIe firmware)
define Package/brcmfmac-firmware-43602a1-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43602-pcie.ap.bin \
		$(1)/lib/firmware/brcm/brcmfmac43602-pcie.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43602a1-pcie))

Package/brcmfmac-firmware-4366b1-pcie = $(call Package/firmware-default,Broadcom 4366b1 FullMAC PCIe firmware)
define Package/brcmfmac-firmware-4366b1-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4366b-pcie.bin \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4366b1-pcie))

Package/brcmfmac-firmware-4329-sdio = $(call Package/firmware-default,Broadcom BCM4329 FullMac SDIO firmware)
define Package/brcmfmac-firmware-4329-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4329-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4329-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4329-sdio))

Package/brcmfmac-firmware-43362-sdio = $(call Package/firmware-default,Broadcom BCM43362 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43362-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43362-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43362-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43362-sdio))

Package/brcmfmac-firmware-43430-sdio = $(call Package/firmware-default,Broadcom BCM43430 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43430-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43430-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43430-sdio))

Package/brcmfmac-firmware-usb = $(call Package/firmware-default,Broadcom BCM43xx fullmac USB firmware)
define Package/brcmfmac-firmware-usb/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43236b.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43143.bin \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-usb))

Package/brcmsmac-firmware = $(call Package/firmware-default,Broadcom BCM43xx softmac PCIe firmware)
define Package/brcmsmac-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/$(PKG_LINUX_FIRMWARE_SUBDIR)/brcm/bcm43xx-0.fw \
		$(PKG_BUILD_DIR)/$(PKG_LINUX_FIRMWARE_SUBDIR)/brcm/bcm43xx_hdr-0.fw \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmsmac-firmware))

Package/bnx2-firmware = $(call Package/firmware-default,Broadcom BCM5706/5708/5709/5716 firmware)
define Package/bnx2-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/bnx2
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/bnx2/* \
		$(1)/lib/firmware/bnx2/
endef
$(eval $(call BuildPackage,bnx2-firmware))
