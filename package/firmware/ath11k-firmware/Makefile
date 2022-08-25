#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ath11k-firmware
PKG_SOURCE_DATE:=2022-04-27
PKG_SOURCE_VERSION:=9a7702977b9f3e06548b2b86fe99f2552b7e7556
PKG_MIRROR_HASH:=da9c25ac3d3e0427fc233541b627b738c22d91a8551d8b2355eef5f91d7f1f47
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kvalo/ath11k-firmware.git

PKG_MAINTAINER:=Robert Marko <robimarko@gmail.com>

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
endef

define Package/ath11k-firmware-ipq8074
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for IPQ8074 devices
endef

define Package/ath11k-firmware-qca6390
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for QCA6390 devices
endef

define Package/ath11k-firmware-qcn9074
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for QCN9074 devices
endef

define Package/ath11k-firmware-wcn6855
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for WCN6855 devices
endef

define Build/Compile

endef

define Package/ath11k-firmware-ipq6018/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/IPQ6018/hw1.0
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ6018
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath11k/IPQ6018/hw1.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/hw1.0/2.5.0.1/WLAN.HK.2.5.0.1-01208-QCAHKSWPL_SILICONZ-1/* \
		$(1)/lib/firmware/IPQ6018/
endef

define Package/ath11k-firmware-ipq8074/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/IPQ8074/hw2.0
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ8074
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath11k/IPQ8074/hw2.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ8074/hw2.0/2.5.0.1/WLAN.HK.2.5.0.1-01208-QCAHKSWPL_SILICONZ-1/* \
		$(1)/lib/firmware/IPQ8074/
endef

define Package/ath11k-firmware-qca6390/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCA6390/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA6390/hw2.0/1.0.1/WLAN.HST.1.0.1-05266-QCAHSTSWPLZ_V2_TO_X86-1/* \
		$(1)/lib/firmware/ath11k/QCA6390/hw2.0/
endef

define Package/ath11k-firmware-qcn9074/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCN9074/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCN9074/hw1.0/2.5.0.1/WLAN.HK.2.5.0.1-01208-QCAHKSWPL_SILICONZ-1/* \
		$(1)/lib/firmware/ath11k/QCN9074/hw1.0/
endef

define Package/ath11k-firmware-wcn6855/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/WCN6855/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/WCN6855/hw2.0/1.1/WLAN.HSP.1.1-03125-QCAHSPSWPL_V1_V2_SILICONZ_LITE-3.6510.7/* \
		$(1)/lib/firmware/ath11k/WCN6855/hw2.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/WCN6855/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath11k/WCN6855/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/WCN6855/hw2.0/regdb.bin \
		$(1)/lib/firmware/ath11k/WCN6855/hw2.0/regdb.bin
	$(LN) $(1)/lib/firmware/ath11k/WCN6855/hw2.0 $(1)/lib/firmware/ath11k/WCN6855/hw2.1
endef

$(eval $(call BuildPackage,ath11k-firmware-ipq6018))
$(eval $(call BuildPackage,ath11k-firmware-ipq8074))
$(eval $(call BuildPackage,ath11k-firmware-qca6390))
$(eval $(call BuildPackage,ath11k-firmware-qcn9074))
$(eval $(call BuildPackage,ath11k-firmware-wcn6855))
