include $(TOPDIR)/rules.mk

PKG_NAME:=pcat-firmware
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/pcat-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Photonicat WiFi Firmware
  DEPENDS:=@TARGET_rockchip_armv8
  PROVIDES:=ath10k-board-qca9377-sdio ath10k-firmware-qca9377-sdio
  CONFLICTS:=ath10k-board-qca9377-sdio ath10k-firmware-qca9377-sdio
endef

define Build/Compile
endef

define Package/pcat-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		./files/board.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/board.bin
	$(INSTALL_DATA) \
		./files/firmware-sdio-5.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/firmware-sdio-5.bin
endef

$(eval $(call BuildPackage,pcat-firmware))
