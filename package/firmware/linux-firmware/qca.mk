Package/ar3k-firmware = $(call Package/firmware-default,Firmware for Qualcomm/Atheros AR3011/AR3012 Bluetooth ICs,,LICENSE.QualcommAtheros_ar3k)
define Package/ar3k-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ar3k
	$(CP) \
		$(PKG_BUILD_DIR)/ar3k/*.dfu \
		$(1)/lib/firmware/ar3k
	$(INSTALL_DIR) $(1)/lib/firmware/qca
	$(CP) \
		$(PKG_BUILD_DIR)/qca/*.bin \
		$(1)/lib/firmware/qca
endef
$(eval $(call BuildPackage,ar3k-firmware))


Package/ath6k-firmware = $(call Package/firmware-default,Firmware for Qualcomm/Atheros AR600x SDIO WiFi ICs)
define Package/ath6k-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath6k
	$(CP) \
		$(PKG_BUILD_DIR)/ath6k/* \
		$(1)/lib/firmware/ath6k
endef
$(eval $(call BuildPackage,ath6k-firmware))

Package/ath9k-htc-firmware = $(call Package/firmware-default,Firmware for Qualcomm/Atheros AR9271/AR7010 USB WiFi adapters,,LICENCE.open-ath9k-htc-firmware)
define Package/ath9k-htc-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath9k_htc
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath9k_htc/htc_9271-1.4.0.fw \
		$(PKG_BUILD_DIR)/ath9k_htc/htc_7010-1.4.0.fw \
		$(1)/lib/firmware/ath9k_htc
endef
$(eval $(call BuildPackage,ath9k-htc-firmware))

Package/carl9170-firmware = $(call Package/firmware-default,Firmware for Qualcomm/Atheros AR9170 USB WiFi adapters,,carl9170fw/GPL)
define Package/carl9170-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/carl9170-1.fw $(1)/lib/firmware
endef
$(eval $(call BuildPackage,carl9170-firmware))

Package/wil6210-firmware = $(call Package/firmware-default,Firmware for Qualcomm Atheros Wilocity 60GHz band 802.11ad WiGig ICs)
define Package/wil6210-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wil6210.fw $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wil6210.brd $(1)/lib/firmware
endef
$(eval $(call BuildPackage,wil6210-firmware))
