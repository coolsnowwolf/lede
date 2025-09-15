Package/edgeport-firmware = $(call Package/firmware-default,USB Inside Out Edgeport Serial Driver firmware)
define Package/edgeport-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/edgeport
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/edgeport/boot.fw \
		$(PKG_BUILD_DIR)/edgeport/boot2.fw \
		$(PKG_BUILD_DIR)/edgeport/down.fw \
		$(PKG_BUILD_DIR)/edgeport/down2.fw \
		$(1)/lib/firmware/edgeport
endef

$(eval $(call BuildPackage,edgeport-firmware))
