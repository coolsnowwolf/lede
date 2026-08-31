Package/aeonsemi-as21xxx-firmware = $(call Package/firmware-default,Aeonsemi AS21xxx Ethernet PHY firmware,,LICENSE.aeonsemi)
define Package/aeonsemi-as21xxx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/aeonsemi
	$(CP) \
		$(PKG_BUILD_DIR)/aeonsemi/as21x1x_fw.bin \
		$(1)/lib/firmware
endef

$(eval $(call BuildPackage,aeonsemi-as21xxx-firmware))
