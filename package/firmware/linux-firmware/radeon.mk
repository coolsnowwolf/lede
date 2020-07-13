Package/radeon-firmware = $(call Package/firmware-default,Radeon Video Driver firmware)
define Package/radeon-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/radeon
	$(CP) \
		$(PKG_BUILD_DIR)/radeon/*.bin \
		$(1)/lib/firmware/radeon
endef

$(eval $(call BuildPackage,radeon-firmware))
