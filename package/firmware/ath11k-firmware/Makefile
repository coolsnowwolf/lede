include $(TOPDIR)/rules.mk

PKG_NAME:=ath11k-firmware
PKG_SOURCE_DATE:=2020-06-26
PKG_SOURCE_VERSION:=ac35049edfa9e59fc0aa9988ba12ea91d28241f5
PKG_MIRROR_HASH:=a5bbe673c7714447df3197de2bc86a09b13d402b89f7f9a813c4b400d897eb9e
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kvalo/ath11k-firmware.git

PKG_MAINTAINER:=Robert Marko <robert.marko@sartura.hr>

include $(INCLUDE_DIR)/package.mk

define Package/ath11k-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=
endef

define Package/ath11k-firmware-ipq6018
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for IPQ6018 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath11k-firmware-ipq8074
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for IPQ8074 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath11k-firmware-qca6390
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for QCA6390 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Build/Compile

endef

define Package/ath11k-firmware-ipq6018/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ6018
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/hw1.0/board-2.bin \
		$(1)/lib/firmware/IPQ6018/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/hw1.0/2.1.0.1/WLAN.HK.2.1.0.1-01238-QCAHKSWPL_SILICONZ-2/* \
		$(1)/lib/firmware/IPQ6018
endef

define Package/ath11k-firmware-ipq8074/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ8074
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/hw2.0/board-2.bin \
		$(1)/lib/firmware/IPQ8074/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/hw2.0/2.1.0.1/WLAN.HK.2.1.0.1-01238-QCAHKSWPL_SILICONZ-2/* \
		$(1)/lib/firmware/IPQ8074
endef

define Package/ath11k-firmware-qca6390/install
	$(INSTALL_DIR) $(1)/lib/firmware/QCA6390
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA6390/hw2.0/WLAN.HST.1.0.1-01740-QCAHSTSWPLZ_V2_TO_X86-1/* \
		$(1)/lib/firmware/QCA6390
endef

$(eval $(call BuildPackage,ath11k-firmware-ipq6018))
$(eval $(call BuildPackage,ath11k-firmware-ipq8074))
$(eval $(call BuildPackage,ath11k-firmware-qca6390))
