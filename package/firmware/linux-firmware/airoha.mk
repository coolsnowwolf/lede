Package/en8811h-firmware = $(call Package/firmware-default,Airoha EN8811H PHY firmware)
define Package/en8811h-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/airoha
	$(CP) \
		$(PKG_BUILD_DIR)/airoha/*.bin \
		$(1)/lib/firmware/airoha
endef
$(eval $(call BuildPackage,en8811h-firmware))
