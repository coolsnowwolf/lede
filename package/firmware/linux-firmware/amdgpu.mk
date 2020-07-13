Package/amdgpu-firmware = $(call Package/firmware-default,AMDGPU Video Driver firmware)
define Package/amdgpu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/amdgpu
	$(CP) \
		$(PKG_BUILD_DIR)/amdgpu/*.bin \
		$(1)/lib/firmware/amdgpu
endef

$(eval $(call BuildPackage,amdgpu-firmware))
