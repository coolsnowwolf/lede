Package/airoha-en8811h-firmware = $(call Package/firmware-default,Airoha EN8811H 2.5G Ethernet PHY firmware,,LICENSE.airoha)
define Package/airoha-en8811h-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/EthMD32.dm.bin \
		$(PKG_BUILD_DIR)/airoha/EthMD32.DSP.bin \
		$(1)/lib/firmware/airoha
ifneq ($(CONFIG_TARGET_mediatek_filogic),)
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	cat \
		$(PKG_BUILD_DIR)/airoha/EthMD32.dm.bin \
		$(PKG_BUILD_DIR)/airoha/EthMD32.DSP.bin \
		> $(STAGING_DIR_IMAGE)/EthMD32.bin
endif
endef

$(eval $(call BuildPackage,airoha-en8811h-firmware))


Package/airoha-en7581-npu-firmware = $(call Package/firmware-default,Airoha EN7581+MT7992 NPU firmware,,LICENSE.airoha)
define Package/airoha-en7581-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/en7581_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/en7581_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-en7581-npu-firmware))


Package/airoha-en7581-mt7996-npu-firmware = $(call Package/firmware-default,Airoha EN7581+MT7996 NPU firmware,,LICENSE.airoha)
define Package/airoha-en7581-mt7996-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/en7581_MT7996_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/en7581_MT7996_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-en7581-mt7996-npu-firmware))


Package/airoha-an7583-npu-firmware = $(call Package/firmware-default,Airoha AN7583 NPU firmware,,LICENSE.airoha)
define Package/airoha-an7583-npu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/an7583_npu_data.bin \
		$(PKG_BUILD_DIR)/airoha/an7583_npu_rv32.bin \
		$(1)/lib/firmware/airoha
endef

$(eval $(call BuildPackage,airoha-an7583-npu-firmware))
