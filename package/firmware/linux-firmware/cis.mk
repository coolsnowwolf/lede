Package/aircard-pcmcia-firmware = $(call Package/firmware-default,Sierra Wireless Aircard 555/7xx/8x0 firmware)
define Package/aircard-pcmcia-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/cis
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/cis/SW_555_SER.cis \
		$(PKG_BUILD_DIR)/cis/SW_7xx_SER.cis \
		$(PKG_BUILD_DIR)/cis/SW_8xx_SER.cis \
		$(1)/lib/firmware/cis
endef
$(eval $(call BuildPackage,aircard-pcmcia-firmware))
