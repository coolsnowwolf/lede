Package/ath11k-firmware-qca2066 = $(call Package/firmware-default,QCA2066 ath11k firmware)
define Package/ath11k-firmware-qca2066/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCA2066/hw2.1
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath11k/QCA2066/hw2.1/* $(1)/lib/firmware/ath11k/QCA2066/hw2.1/
endef
$(eval $(call BuildPackage,ath11k-firmware-qca2066))

Package/ath11k-firmware-qca6390 = $(call Package/firmware-default,QCA6390 ath11k firmware)
define Package/ath11k-firmware-qca6390/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCA6390/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath11k/QCA6390/hw2.0/* $(1)/lib/firmware/ath11k/QCA6390/hw2.0/
endef
$(eval $(call BuildPackage,ath11k-firmware-qca6390))

Package/ath11k-firmware-wcn6750 = $(call Package/firmware-default,WCN6750 ath11k firmware)
define Package/ath11k-firmware-wcn6750/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/WCN6750/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath11k/WCN6750/hw1.0/board-2.bin $(1)/lib/firmware/ath11k/WCN6750/hw1.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath11k/WCN6750/hw1.0/sc7280/wpss.mbn $(1)/lib/firmware/ath11k/WCN6750/hw1.0/
endef
$(eval $(call BuildPackage,ath11k-firmware-wcn6750))

Package/ath11k-firmware-wcn6855 = $(call Package/firmware-default,WCN6855 ath11k firmware)
define Package/ath11k-firmware-wcn6855/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/WCN6855/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath11k/WCN6855/hw2.0/* $(1)/lib/firmware/ath11k/WCN6855/hw2.0/
	$(LN) ./hw2.0 $(1)/lib/firmware/ath11k/WCN6855/hw2.1
endef
$(eval $(call BuildPackage,ath11k-firmware-wcn6855))
