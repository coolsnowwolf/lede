Package/i915-firmware = $(call Package/firmware-default,Intel GPU Video Driver firmware)
define Package/i915-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	$(CP) \
		$(PKG_BUILD_DIR)/i915/*.bin \
		$(1)/lib/firmware/i915
endef

$(eval $(call BuildPackage,i915-firmware))
