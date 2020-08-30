Package/eip197-mini-firmware = $(call Package/firmware-default,Inside Secure EIP197 mini firmware)
define Package/eip197-mini-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/inside-secure/eip197_minifw
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/inside-secure/eip197_minifw/ifpp.bin \
		$(PKG_BUILD_DIR)/inside-secure/eip197_minifw/ipue.bin \
		$(1)/lib/firmware/inside-secure/eip197_minifw
endef
$(eval $(call BuildPackage,eip197-mini-firmware))
