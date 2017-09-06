Package/mwl8k-firmware = $(call Package/firmware-default,Marvell 8366/8687 firmware)
define Package/mwl8k-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mwl8k
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mwl8k/fmimage_8366_ap-3.fw \
		$(PKG_BUILD_DIR)/mwl8k/fmimage_8366.fw \
		$(PKG_BUILD_DIR)/mwl8k/helper_8366.fw \
		$(PKG_BUILD_DIR)/mwl8k/fmimage_8687.fw \
		$(PKG_BUILD_DIR)/mwl8k/helper_8687.fw \
		$(1)/lib/firmware/mwl8k/
endef
$(eval $(call BuildPackage,mwl8k-firmware))

Package/mwifiex-pcie-firmware = $(call Package/firmware-default,Marvell 8897 firmware)
define Package/mwifiex-pcie-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mrvl/pcie8897_uapsta.bin \
		$(1)/lib/firmware/mrvl/
endef
$(eval $(call BuildPackage,mwifiex-pcie-firmware))

Package/mwifiex-sdio-firmware = $(call Package/firmware-default,Marvell 8887 firmware)
define Package/mwifiex-sdio-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mrvl/sd8887_uapsta.bin \
		$(1)/lib/firmware/mrvl/
endef
$(eval $(call BuildPackage,mwifiex-sdio-firmware))

Package/libertas-usb-firmware = $(call Package/firmware-default,Marvell 8388/8682 USB firmware)
define Package/libertas-usb-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/usb8388_v9.bin \
		$(PKG_BUILD_DIR)/libertas/usb8682.bin \
		$(1)/lib/firmware/libertas/
endef
$(eval $(call BuildPackage,libertas-usb-firmware))

Package/libertas-sdio-firmware = $(call Package/firmware-default,Marvell 8385/8686/8688 SDIO firmware)
define Package/libertas-sdio-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/sd8385_helper.bin \
		$(PKG_BUILD_DIR)/libertas/sd8385.bin \
		$(PKG_BUILD_DIR)/libertas/sd8686_v9_helper.bin \
		$(PKG_BUILD_DIR)/libertas/sd8686_v9.bin \
		$(PKG_BUILD_DIR)/libertas/sd8688_helper.bin \
		$(PKG_BUILD_DIR)/libertas/sd8688.bin \
		$(1)/lib/firmware/libertas
endef
$(eval $(call BuildPackage,libertas-sdio-firmware))

Package/libertas-spi-firmware = $(call Package/firmware-default,Marvell 8686 SPI firmware)
define Package/libertas-spi-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/gspi8686_v9_helper.bin \
		$(PKG_BUILD_DIR)/libertas/gspi8686_v9.bin \
		$(1)/lib/firmware/libertas
endef
$(eval $(call BuildPackage,libertas-spi-firmware))

