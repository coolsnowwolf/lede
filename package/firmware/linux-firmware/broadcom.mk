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

Package/brcmfmac-firmware-4366c0-pcie = $(call Package/firmware-default,Broadcom 4366c0 FullMAC PCIe firmware)
define Package/brcmfmac-firmware-4366c0-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4366c-pcie.bin \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4366c0-pcie))

Package/brcmfmac-firmware-4329-sdio = $(call Package/firmware-default,Broadcom BCM4329 FullMac SDIO firmware)
define Package/brcmfmac-firmware-4329-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4329-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4329-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4329-sdio))

Package/brcmfmac-firmware-43430-sdio-rpi-3b = $(call Package/firmware-default,Broadcom BCM43430 NVRAM for Raspberry Pi 3B)
define Package/brcmfmac-firmware-43430-sdio-rpi-3b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43430-sdio.raspberrypi,3-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,3-model-b.txt
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43430-sdio-rpi-3b))

Package/brcmfmac-firmware-43430-sdio-rpi-zero-w = $(call Package/firmware-default,Broadcom BCM43430 NVRAM for Raspberry Pi Zero W)
define Package/brcmfmac-firmware-43430-sdio-rpi-zero-w/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43430-sdio.raspberrypi,3-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,model-zero-w.txt
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43430-sdio-rpi-zero-w))

Package/brcmfmac-firmware-43430a0-sdio = $(call Package/firmware-default,Broadcom BCM43430a0 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43430a0-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43430a0-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43430a0-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43430a0-sdio))

Package/brcmfmac-firmware-43455-sdio-rpi-3b-plus = $(call Package/firmware-default,Broadcom BCM43455 NVRAM for Raspberry Pi 3B+)
define Package/brcmfmac-firmware-43455-sdio-rpi-3b-plus/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43455-sdio-rpi-3b-plus))

Package/brcmfmac-firmware-43455-sdio-rpi-4b = $(call Package/firmware-default,Broadcom BCM43455 NVRAM for Raspberry Pi 4B)
define Package/brcmfmac-firmware-43455-sdio-rpi-4b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43455-sdio.raspberrypi,4-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,4-model-b.txt
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43455-sdio-rpi-4b))

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

Package/bnx2x-firmware = $(call Package/firmware-default,=QLogic 5771x/578xx firmware)
define Package/bnx2x-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/bnx2x
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/bnx2x/* \
		$(1)/lib/firmware/bnx2x/
endef
$(eval $(call BuildPackage,bnx2x-firmware))

Package/station-p2-firmware = $(call Package/firmware-default,Broadcom FullMac SDIO firmware)
define Package/station-p2-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) ./brcm_firmware/ap6275s/BCM4362A2.hcd $(1)/lib/firmware/brcm/BCM4362A2.hcd
	$(INSTALL_DATA) ./brcm_firmware/ap6275s/clm_bcm43752a2_ag.blob $(1)/lib/firmware/brcm/brcmfmac43752-sdio.clm_blob
	$(INSTALL_DATA) ./brcm_firmware/ap6275s/fw_bcm43752a2_ag_apsta.bin $(1)/lib/firmware/brcm/brcmfmac43752-sdio.firefly,rk3568-roc-pc.bin
	$(INSTALL_DATA) ./brcm_firmware/ap6275s/nvram_ap6275s.txt $(1)/lib/firmware/brcm/brcmfmac43752-sdio.firefly,rk3568-roc-pc.txt
endef
$(eval $(call BuildPackage,station-p2-firmware))

Package/rongpin-king3399-firmware = $(call Package/firmware-default,Broadcom FullMac SDIO firmware)
define Package/rongpin-king3399-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) ./brcm_firmware/ap6356s/brcmfmac4356-sdio.rongpin,king3399.bin $(1)/lib/firmware/brcm/brcmfmac4356-sdio.rongpin,king3399.bin
	$(INSTALL_DATA) ./brcm_firmware/ap6356s/brcmfmac4356-sdio.rongpin,king3399.txt $(1)/lib/firmware/brcm/brcmfmac4356-sdio.rongpin,king3399.txt
endef
$(eval $(call BuildPackage,rongpin-king3399-firmware))
