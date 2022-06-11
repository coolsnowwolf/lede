Package/ath10k-board-qca4019 = $(call Package/firmware-default,ath10k qca4019 board firmware)
define Package/ath10k-board-qca4019/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA4019/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/
endef
$(eval $(call BuildPackage,ath10k-board-qca4019))
Package/ath10k-firmware-qca4019 = $(call Package/firmware-default,ath10k qca4019 firmware,+ath10k-board-qca4019)
define Package/ath10k-firmware-qca4019/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA4019/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca4019))

Package/ath10k-board-qca9377 = $(call Package/firmware-default,ath10k qca9377 board firmware)
define Package/ath10k-board-qca9377/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9377/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/
endef
$(eval $(call BuildPackage,ath10k-board-qca9377))
Package/ath10k-firmware-qca9377 = $(call Package/firmware-default,ath10k qca9377 firmware,+ath10k-board-qca9377)
define Package/ath10k-firmware-qca9377/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9377/hw1.0/firmware-6.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/firmware-6.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9377))

Package/ath10k-board-qca9887 = $(call Package/firmware-default,ath10k qca9887 board firmware)
define Package/ath10k-board-qca9887/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9887/hw1.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef
$(eval $(call BuildPackage,ath10k-board-qca9887))
Package/ath10k-firmware-qca9887 = $(call Package/firmware-default,ath10k qca9887 firmware,+ath10k-board-qca9887)
define Package/ath10k-firmware-qca9887/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9887/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9887))

Package/ath10k-board-qca9888 = $(call Package/firmware-default,ath10k qca9888 board firmware)
define Package/ath10k-board-qca9888/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9888/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
endef
$(eval $(call BuildPackage,ath10k-board-qca9888))
Package/ath10k-firmware-qca9888 = $(call Package/firmware-default,ath10k qca9888 firmware,+ath10k-board-qca9888)
define Package/ath10k-firmware-qca9888/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9888/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9888))

Package/ath10k-board-qca988x = $(call Package/firmware-default,ath10k qca988x board firmware)
define Package/ath10k-board-qca988x/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
endef
$(eval $(call BuildPackage,ath10k-board-qca988x))
Package/ath10k-firmware-qca988x = $(call Package/firmware-default,ath10k qca988x firmware,+ath10k-board-qca988x)
define Package/ath10k-firmware-qca988x/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA988X/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca988x))

Package/ath10k-firmware-qca6174 = $(call Package/firmware-default,ath10k qca6174 firmware)
define Package/ath10k-firmware-qca6174/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA6174/hw2.1
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw2.1/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw2.1/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw2.1/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw2.1/firmware-5.bin
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA6174/hw3.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw3.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw3.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw3.0/firmware-6.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw3.0/firmware-6.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca6174))

Package/ath10k-board-qca99x0 = $(call Package/firmware-default,ath10k qca99x0 board firmware)
define Package/ath10k-board-qca99x0/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA99X0/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
endef
$(eval $(call BuildPackage,ath10k-board-qca99x0))

Package/ath10k-firmware-qca99x0 = $(call Package/firmware-default,ath10k qca99x0 firmware,+ath10k-board-qca99x0)
define Package/ath10k-firmware-qca99x0/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA99X0/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca99x0))

Package/ath10k-board-qca9984 = $(call Package/firmware-default,ath10k qca9984 board firmware)
define Package/ath10k-board-qca9984/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
endef
$(eval $(call BuildPackage,ath10k-board-qca9984))
Package/ath10k-firmware-qca9984 = $(call Package/firmware-default,ath10k qca9984 firmware,+ath10k-board-qca9984)
define Package/ath10k-firmware-qca9984/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9984/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9984))
