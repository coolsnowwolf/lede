Package/mlxsw_spectrum-firmware = $(call Package/firmware-default,Mellanox Spectrum firmware)
define Package/mlxsw_spectrum-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mellanox
	$(CP) \
		$(PKG_BUILD_DIR)/mellanox/mlxsw_spectrum-*.mfa2 \
		$(1)/lib/firmware/mellanox
endef

$(eval $(call BuildPackage,mlxsw_spectrum-firmware))

Package/mlxsw_spectrum2-firmware = $(call Package/firmware-default,Mellanox Spectrum-2 firmware)
define Package/mlxsw_spectrum2-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mellanox
	$(CP) \
		$(PKG_BUILD_DIR)/mellanox/mlxsw_spectrum2-*.mfa2 \
		$(1)/lib/firmware/mellanox
endef

$(eval $(call BuildPackage,mlxsw_spectrum2-firmware))

Package/mlxsw_spectrum3-firmware = $(call Package/firmware-default,Mellanox Spectrum-3 firmware)
define Package/mlxsw_spectrum3-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mellanox
	$(CP) \
		$(PKG_BUILD_DIR)/mellanox/mlxsw_spectrum3-*.mfa2 \
		$(1)/lib/firmware/mellanox
endef

$(eval $(call BuildPackage,mlxsw_spectrum3-firmware))

Package/mlxsw_spectrum4-firmware = $(call Package/firmware-default,Mellanox Spectrum-4 firmware)
define Package/mlxsw_spectrum4-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mellanox
	$(CP) \
		$(PKG_BUILD_DIR)/mellanox/mlxsw_spectrum4-*.mfa2 \
		$(1)/lib/firmware/mellanox
endef

$(eval $(call BuildPackage,mlxsw_spectrum4-firmware))
