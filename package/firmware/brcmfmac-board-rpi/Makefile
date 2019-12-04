include $(TOPDIR)/rules.mk

PKG_NAME:=brcmfmac-board-rpi
PKG_VERSION:=2018-03-12
PKG_RELEASE:=86e88fbf0345da49555d0ec34c80b4fbae7d0cd3
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

BRCMFMAC434XX_SDIO_URL:=@GITHUB/RPi-Distro/firmware-nonfree/$(PKG_RELEASE)/brcm/
BRCMFMAC43455_SDIO_FILE:=brcmfmac43455-sdio-$(PKG_RELEASE)

define Download/brcmfmac43455_sdio_clm_blob
  FILE:=$(BRCMFMAC43455_SDIO_FILE).clm_blob
  URL:=$(BRCMFMAC434XX_SDIO_URL)
  URL_FILE:=brcmfmac43455-sdio.clm_blob
  HASH:=8e2250518bc789e53109728c3c0a6124bc3801a75a1cb4966125753cf1f0252e
endef
$(eval $(call Download,brcmfmac43455_sdio_clm_blob))

define Package/brcmfmac-board-rpi/Default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://github.com/RPi-Distro/firmware-nonfree/
  DEPENDS:=@TARGET_brcm2708
  PKGARCH:=all
endef

define Package/brcmfmac-firmware-43455-clm_blob
  $(call Package/brcmfmac-board-rpi/Default)
  TITLE:=BCM43455 RPi SDIO Localisation Blob
endef

define Package/brcmfmac-firmware-43455-clm_blob/description
  Localisation blob for the Broadcom BCM43455 on RPi boards.
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp $(DL_DIR)/$(BRCMFMAC43455_SDIO_FILE).clm_blob $(PKG_BUILD_DIR)/brcmfmac43455-sdio.clm_blob
endef

define Build/Compile
	true
endef

define Package/brcmfmac-firmware-43455-clm_blob/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/brcmfmac43455-sdio.clm_blob $(1)/lib/firmware/brcm/
endef

$(eval $(call BuildPackage,brcmfmac-firmware-43455-clm_blob))
