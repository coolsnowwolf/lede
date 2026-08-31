Package/mwl8k-firmware = $(call Package/firmware-default,Firmware for Marvell 88W8366/88W8687 TopDog PCIe WiFi ICs,,LICENCE.Marvell)
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

Package/mwifiex-pcie-firmware = $(call Package/firmware-default,Firmware for Marvell/NXP 88W8897 PCIe Bluetooth/NFC/WiFi ICs,,LICENCE.Marvell)
define Package/mwifiex-pcie-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mrvl/pcie8897_uapsta.bin \
		$(1)/lib/firmware/mrvl/
endef
$(eval $(call BuildPackage,mwifiex-pcie-firmware))

Package/mwifiex-sdio-firmware = $(call Package/firmware-default,Firmware for Marvell/NXP 88W8887/88W8997 SDIO Bluetooth/WiFi ICs,,LICENCE.Marvell)
define Package/mwifiex-sdio-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mrvl/sd8887_uapsta.bin \
		$(PKG_BUILD_DIR)/mrvl/sdsd8997_combo_v4.bin \
		$(1)/lib/firmware/mrvl/
	ln -s ../mrvl/sdsd8997_combo_v4.bin $(1)/lib/firmware/mrvl/sd8997_uapsta.bin
endef
$(eval $(call BuildPackage,mwifiex-sdio-firmware))

Package/libertas-usb-firmware = $(call Package/firmware-default,Firmware for Marvell 88W8388/88W8682 USB WiFi ICs,,LICENCE.Marvell)
define Package/libertas-usb-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/usb8388_v9.bin \
		$(PKG_BUILD_DIR)/libertas/usb8682.bin \
		$(1)/lib/firmware/libertas/
endef
$(eval $(call BuildPackage,libertas-usb-firmware))

Package/libertas-sdio-firmware = $(call Package/firmware-default,Marvell 8385/8686/8688 SDIO firmware,,LICENCE.Marvell)
define Package/libertas-sdio-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/sd8385_helper.bin \
		$(PKG_BUILD_DIR)/libertas/sd8385.bin \
		$(PKG_BUILD_DIR)/libertas/sd8686_v9_helper.bin \
		$(PKG_BUILD_DIR)/libertas/sd8686_v9.bin \
		$(1)/lib/firmware/libertas
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mrvl/sd8688_helper.bin \
		$(PKG_BUILD_DIR)/mrvl/sd8688.bin \
		$(1)/lib/firmware/mrvl
	ln -s ../mrvl/sd8688_helper.bin $(1)/lib/firmware/libertas/sd8688_helper.bin
	ln -s ../mrvl/sd8688.bin $(1)/lib/firmware/libertas/sd8688.bin
endef
$(eval $(call BuildPackage,libertas-sdio-firmware))

Package/libertas-spi-firmware = $(call Package/firmware-default,Marvell 8686 SPI firmware,,LICENCE.Marvell)
define Package/libertas-spi-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/libertas
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/libertas/gspi8686_v9_helper.bin \
		$(PKG_BUILD_DIR)/libertas/gspi8686_v9.bin \
		$(1)/lib/firmware/libertas
endef
$(eval $(call BuildPackage,libertas-spi-firmware))

Package/qed-firmware = $(call Package/firmware-default,QLogic FastLinQ 41xxx firmware)
define Package/qed-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/qed
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/qed/* \
		$(1)/lib/firmware/qed/
endef
$(eval $(call BuildPackage,qed-firmware))
