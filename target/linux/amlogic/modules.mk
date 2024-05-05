define Package/brcmfmac-firmware-43455-sdio-phicomm-n1
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Broadcom BCM43455 firmware for Phicomm N1
  VERSION:=1.0.0
  DEPENDS:=+brcmfmac-firmware-43455-sdio +brcmfmac-nvram-43455-sdio
endef

define Package/brcmfmac-firmware-43455-sdio-phicomm-n1/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) brcmfmac43455-sdio.raspberrypi,4-model-b.txt $(1)/lib/firmware/brcm/brcmfmac43455-sdio.phicomm,n1.txt
	$(LN) brcmfmac43455-sdio.bin $(1)/lib/firmware/brcm/brcmfmac43455-sdio.phicomm,n1.bin
	$(LN) brcmfmac43455-sdio.clm_blob $(1)/lib/firmware/brcm/brcmfmac43455-sdio.phicomm,n1.clm_blob
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43455-sdio-phicomm-n1))
