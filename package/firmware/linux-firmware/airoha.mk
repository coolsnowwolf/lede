Package/airoha-en8811h-firmware = $(call Package/firmware-default,Airoha EN8811H 2.5G Ethernet PHY firmware)
define Package/airoha-en8811h-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/EthMD32.dm.bin \
		$(PKG_BUILD_DIR)/airoha/EthMD32.DSP.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-en8811h-firmware))


Package/airoha-en7581-npu-firmware = $(call Package/firmware-default,Airoha EN7581 NPU firmware)
define Package/airoha-en7581-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/en7581_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/en7581_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-en7581-npu-firmware))


Package/airoha-en7581-mt7996-npu-firmware = $(call Package/firmware-default,Airoha EN7581+MT7996 NPU firmware)
define Package/airoha-en7581-mt7996-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/en7581_MT7996_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/en7581_MT7996_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-en7581-mt7996-npu-firmware))


Package/airoha-an7583-npu-firmware = $(call Package/firmware-default,Airoha AN7583 NPU firmware)
define Package/airoha-an7583-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/an7583_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/an7583_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-an7583-npu-firmware))
