Package/rs9113-firmware = $(call Package/firmware-default,RedPine Signals rs9113 firmware)
define Package/rs9113-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rsi
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rsi/rs9113_wlan_qspi.rps $(1)/lib/firmware/rsi
endef
$(eval $(call BuildPackage,rs9113-firmware))
