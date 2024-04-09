include $(TOPDIR)/rules.mk

PKG_NAME:=photonicat-firmware
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/photonicat-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Photonicat WiFi Firmware
  DEPENDS:=@TARGET_rockchip_armv8
endef

define Build/Compile
endef

define Package/photonicat-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		./files/board.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/board.bin
	$(INSTALL_DATA) \
		./files/firmware-sdio-5.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/firmware-sdio-5.bin
endef

$(eval $(call BuildPackage,photonicat-firmware))
