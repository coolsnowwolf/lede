Package/wl12xx-firmware = $(call Package/firmware-default,TI WL12xx firmware)
define Package/wl12xx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ti-connectivity
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-mr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-plt.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl127x-fw-5-sr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl1271-nvs.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-mr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-plt.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-fw-5-sr.bin \
		$(PKG_BUILD_DIR)/ti-connectivity/wl128x-nvs.bin \
		$(1)/lib/firmware/ti-connectivity
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

