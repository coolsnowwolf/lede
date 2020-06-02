Package/wl12xx-firmware = $(call Package/firmware-default,TI WL12xx firmware)
define Package/wl12xx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ti-connectivity
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-mr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-plt.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-sr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-nvs.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-mr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-plt.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-sr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-nvs.bin \
		$(1)/lib/firmware/ti-connectivity
	ln -s wl127x-nvs.bin $(1)/lib/firmware/ti-connectivity/wl1271-nvs.bin
endef
$(eval $(call BuildPackage,wl12xx-firmware))

Package/wl18xx-firmware = $(call Package/firmware-default,TI WL18xx firmware)
define Package/wl18xx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ti-connectivity
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ti-connectivity/wl18xx-fw-4.bin \
		$(1)/lib/firmware/ti-connectivity
endef
$(eval $(call BuildPackage,wl18xx-firmware))

Package/ti-3410-firmware = $(call Package/firmware-default,TI 3410 firmware)
define Package/ti-3410-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ti_3410.fw $(1)/lib/firmware
endef
$(eval $(call BuildPackage,ti-3410-firmware))

Package/ti-5052-firmware = $(call Package/firmware-default,TI 5052 firmware)
define Package/ti-5052-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ti_5052.fw $(1)/lib/firmware
endef
$(eval $(call BuildPackage,ti-5052-firmware))
