Package/mlxsw_spectrum-firmware = $(call Package/firmware-default,Mellanox Spectrum firmware)
define Package/mlxsw_spectrum-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mellanox
	$(CP) \
		$(PKG_BUILD_DIR)/mellanox/mlxsw_spectrum-*.mfa2 \
		$(1)/lib/firmware/mellanox
endef

$(eval $(call BuildPackage,mlxsw_spectrum-firmware))
