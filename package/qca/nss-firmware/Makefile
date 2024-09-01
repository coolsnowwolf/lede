#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nss-firmware
PKG_SOURCE_DATE:=2022-07-14
PKG_SOURCE_VERSION:=ade6bff594377c9d9c79b45e39bf104303d919bc
PKG_MIRROR_HASH:=99ca44dd0733cff569308550c6c74febb0e7a03093b14df092d0f53362189647
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/quic/qca-sdk-nss-fw.git

PKG_LICENSE_FILES:=LICENSE.md

PKG_MAINTAINER:=Robert Marko <robimarko@gmail.com>

include $(INCLUDE_DIR)/package.mk

VERSION_PATH=$(PKG_BUILD_DIR)/QCA_Networking_2022.SPF_12.0.0/ED1

define Package/nss-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=@TARGET_qualcommax
endef

define Package/nss-firmware-ipq5018
$(Package/nss-firmware-default)
  TITLE:=NSS firmware for IPQ5018 devices
  NSS_ARCHIVE:=$(VERSION_PATH)/IPQ5018.ATH.12.0.0/BIN-NSS.FW.12.1-022-MP.R.tar.bz2
endef

define Package/nss-firmware-ipq6018
$(Package/nss-firmware-default)
  TITLE:=NSS firmware for IPQ6018 devices
  NSS_ARCHIVE:=$(VERSION_PATH)/IPQ6018.ATH.12.0.0/BIN-NSS.FW.12.1-022-CP.R.tar.bz2
endef

define Package/nss-firmware-ipq8074
$(Package/nss-firmware-default)
  TITLE:=NSS firmware for IPQ8074 devices
  NSS_ARCHIVE:=$(VERSION_PATH)/IPQ8074.ATH.12.0.0/BIN-NSS.FW.12.1-022-HK.R.tar.bz2
endef

define Build/Compile
	true
endef

define Package/nss-firmware-ipq5018/install
	mkdir -p $(PKG_BUILD_DIR)/IPQ5018
	$(TAR) -C $(PKG_BUILD_DIR)/IPQ5018 -xf $(NSS_ARCHIVE) --strip-components=1
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ5018/retail_router0.bin \
		$(1)/lib/firmware/qca-nss0.bin
endef

define Package/nss-firmware-ipq6018/install
	mkdir -p $(PKG_BUILD_DIR)/IPQ6018
	$(TAR) -C $(PKG_BUILD_DIR)/IPQ6018 -xf $(NSS_ARCHIVE) --strip-components=1
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/retail_router0.bin \
		$(1)/lib/firmware/qca-nss0.bin
endef

define Package/nss-firmware-ipq8074/install
	mkdir -p $(PKG_BUILD_DIR)/IPQ8074
	$(TAR) -C $(PKG_BUILD_DIR)/IPQ8074 -xf $(NSS_ARCHIVE) --strip-components=1
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/retail_router0.bin \
		$(1)/lib/firmware/qca-nss0.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/retail_router1.bin \
		$(1)/lib/firmware/qca-nss1.bin
endef

$(eval $(call BuildPackage,nss-firmware-ipq5018))
$(eval $(call BuildPackage,nss-firmware-ipq6018))
$(eval $(call BuildPackage,nss-firmware-ipq8074))
