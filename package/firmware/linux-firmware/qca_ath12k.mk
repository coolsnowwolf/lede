Package/ath12k-firmware-wcn7850 = $(call Package/firmware-default,WCN7850 ath12k firmware)
define Package/ath12k-firmware-wcn7850/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath12k/WCN7850/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath12k/WCN7850/hw2.0/* $(1)/lib/firmware/ath12k/WCN7850/hw2.0/
endef
$(eval $(call BuildPackage,ath12k-firmware-wcn7850))

Package/ath12k-firmware-qcn9274 = $(call Package/firmware-default,QCN9274 ath12k firmware)
define Package/ath12k-firmware-qcn9274/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath12k/QCN9274/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath12k/QCN9274/hw2.0/* $(1)/lib/firmware/ath12k/QCN9274/hw2.0/
endef
$(eval $(call BuildPackage,ath12k-firmware-qcn9274))
