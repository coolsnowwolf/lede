Package/ar3k-firmware = $(call Package/firmware-default,ath3k firmware)
define Package/ar3k-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ar3k
	$(CP) \
		$(PKG_BUILD_DIR)/ar3k/*.dfu \
		$(1)/lib/firmware/ar3k
endef
$(eval $(call BuildPackage,ar3k-firmware))

Package/ath9k-htc-firmware = $(call Package/firmware-default,AR9271/AR7010 firmware)
define Package/ath9k-htc-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath9k_htc
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath9k_htc/htc_9271-1.4.0.fw \
		$(PKG_BUILD_DIR)/ath9k_htc/htc_7010-1.4.0.fw \
		$(1)/lib/firmware/ath9k_htc
endef
$(eval $(call BuildPackage,ath9k-htc-firmware))

Package/carl9170-firmware = $(call Package/firmware-default,AR9170 firmware)
define Package/carl9170-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/carl9170-1.fw $(1)/lib/firmware
endef
$(eval $(call BuildPackage,carl9170-firmware))
