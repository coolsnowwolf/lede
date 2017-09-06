Package/mt7601u-firmware = $(call Package/firmware-default,MediaTek MT7601U firmware)
define Package/mt7601u-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/mt7601u.bin \
		$(1)/lib/firmware
endef
$(eval $(call BuildPackage,mt7601u-firmware))

Package/rt2800-pci-firmware = $(call Package/firmware-default,Ralink RT28xx/3xxx PCI/SoC firmware)
define Package/rt2800-pci-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/rt2860.bin \
		$(PKG_BUILD_DIR)/rt3290.bin \
		$(1)/lib/firmware
endef
$(eval $(call BuildPackage,rt2800-pci-firmware))

Package/rt2800-usb-firmware = $(call Package/firmware-default,Ralink RT28xx/3xxx USB firmware)
define Package/rt2800-usb-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rt2870.bin $(1)/lib/firmware/
endef
$(eval $(call BuildPackage,rt2800-usb-firmware))

Package/rt61-pci-firmware = $(call Package/firmware-default,Ralink RT2561 firmware)
define Package/rt61-pci-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/rt2561.bin \
		$(PKG_BUILD_DIR)/rt2561s.bin \
		$(PKG_BUILD_DIR)/rt2661.bin \
		$(1)/lib/firmware/
endef
$(eval $(call BuildPackage,rt61-pci-firmware))

Package/rt73-usb-firmware = $(call Package/firmware-default,Ralink RT2573 firmware)
define Package/rt73-usb-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rt73.bin $(1)/lib/firmware/
endef
$(eval $(call BuildPackage,rt73-usb-firmware))
