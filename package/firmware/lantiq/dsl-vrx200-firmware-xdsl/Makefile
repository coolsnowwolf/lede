# Copyright (C) 2015 OpenWrt.org
# Copyright (C) 2015-2016 Lantiq Beteiligungs GmbH & Co KG.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=dsl_vr9_firmware_xdsl
PKG_VERSION:=05.08.01.08.01.06_05.08.00.0B.01.01_osc
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@OPENWRT
PKG_HASH:=44cd94130571fe42dfa8f0f9d44597d104e9e77962617fe38646b7a0b4184a2b
PKG_BUILD_DEPENDS:=bsdiff/host

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

PKG_B_NAME:=dsl_vr9_firmware_xdsl
PKG_B_VERSION:=05.07.09.09.00.06_05.07.04.04.00.02_osc
PKG_B_SOURCE:=$(PKG_B_NAME)-$(PKG_B_VERSION).tar.gz

ANNEX_A_VER:=581816_580B11
ANNEX_B_VER:=579906_574402

define Download/dsl_vr9_firmware_xdsl_b
  FILE:=$(PKG_B_SOURCE)
  URL:=$(PKG_SOURCE_URL)
  HASH:=275c55e870205a5a75510d3ef3d3fb6b60010effebf4b2d1fbc72ffd46e855c0
endef
$(eval $(call Download,dsl_vr9_firmware_xdsl_b))


define Package/dsl-vrx200-firmware-xdsl-a
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=VRX200 / VR9 CPE xDSL Annex A firmware
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq_xrx200
endef

define Package/dsl-vrx200-firmware-xdsl-a/description
	VRX200 / VR9 CPE VDSL and ADSL Annex A firmware
endef

define Package/dsl-vrx200-firmware-xdsl-b
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=VRX200 / VR9 CPE xDSL Annex B firmware
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq_xrx200
endef

define Package/dsl-vrx200-firmware-xdsl-b/description
	VRX200 / VR9 CPE VDSL and ADSL Annex B firmware
endef

define Package/dsl-vrx200-firmware-xdsl-a-patch
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=VRX200 / VR9 CPE xDSL Annex B to Annex A firmware patch
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq_xrx200 +dsl-vrx200-firmware-xdsl-b +bspatch
endef

define Package/dsl-vrx200-firmware-xdsl-a-patch/description
	Patch which between the Annex A and Annex B firmware to create the Annex A firmware.
endef

define Package/dsl-vrx200-firmware-xdsl-b-patch
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=VRX200 / VR9 CPE xDSL Annex A to Annex B firmware patch
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq_xrx200 +dsl-vrx200-firmware-xdsl-a +bspatch
endef

define Package/dsl-vrx200-firmware-xdsl-b-patch/description
	Patch which between the Annex A and Annex B firmware to create the Annex B firmware.
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(TAR) -C $(PKG_BUILD_DIR) -xzf $(DL_DIR)/$(PKG_SOURCE)
	$(TAR) -C $(PKG_BUILD_DIR) -xzf $(DL_DIR)/$(PKG_B_SOURCE)
endef

define Build/Compile
	bsdiff \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_A_VER).bin \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_B_VER).bin \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_A_VER)_to_$(ANNEX_B_VER).bspatch
	bsdiff \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_B_VER).bin \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_A_VER).bin \
		$(PKG_BUILD_DIR)/xcpe_$(ANNEX_B_VER)_to_$(ANNEX_A_VER).bspatch
endef

define Package/dsl-vrx200-firmware-xdsl-a/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/LICENSE $(1)/lib/firmware/xcpe_$(ANNEX_A_VER).LICENSE
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/xcpe_$(ANNEX_A_VER).bin $(1)/lib/firmware/
	ln -s xcpe_$(ANNEX_A_VER).bin $(1)/lib/firmware/lantiq-vrx200-a.bin
endef

define Package/dsl-vrx200-firmware-xdsl-b/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/LICENSE $(1)/lib/firmware/xcpe_$(ANNEX_B_VER).LICENSE
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/xcpe_$(ANNEX_B_VER).bin $(1)/lib/firmware/
	ln -s xcpe_$(ANNEX_B_VER).bin $(1)/lib/firmware/lantiq-vrx200-b.bin
endef

define Package/dsl-vrx200-firmware-xdsl-a-patch/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/xcpe_$(ANNEX_B_VER)_to_$(ANNEX_A_VER).bspatch $(1)/lib/firmware/
	ln -s xcpe_$(ANNEX_B_VER)_to_$(ANNEX_A_VER).bspatch $(1)/lib/firmware/lantiq-vrx200-b-to-a.bspatch
endef

define Package/dsl-vrx200-firmware-xdsl-b-patch/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/xcpe_$(ANNEX_A_VER)_to_$(ANNEX_B_VER).bspatch $(1)/lib/firmware/
	ln -s xcpe_$(ANNEX_A_VER)_to_$(ANNEX_B_VER).bspatch $(1)/lib/firmware/lantiq-vrx200-a-to-b.bspatch
endef

$(eval $(call BuildPackage,dsl-vrx200-firmware-xdsl-a))
$(eval $(call BuildPackage,dsl-vrx200-firmware-xdsl-b))
$(eval $(call BuildPackage,dsl-vrx200-firmware-xdsl-a-patch))
$(eval $(call BuildPackage,dsl-vrx200-firmware-xdsl-b-patch))
