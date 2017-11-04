#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-firmware
PKG_SOURCE_DATE:=2017-03-29
PKG_SOURCE_VERSION:=956e2609b7e42c8c710bba10ef925a5be1be5137
PKG_MIRROR_HASH:=25f724ff38c830281b3efba4a4ddffaae0c4bd8fea0f4c1061591229ff05535b
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kvalo/ath10k-firmware.git

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

define Package/ath10k-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=
endef

define Package/ath10k-firmware-qca9887
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA9887 devices
endef

define Package/ath10k-firmware-qca9888
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA9888 devices
endef

define Package/ath10k-firmware-qca9887-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k-CT firmware for QCA9887 devices
endef

define Package/ath10k-firmware-qca988x
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA988x devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

CT_FIRMWARE_FILE = $(1)-$($(1)_FIRMWARE_FILE_CT)

define Download/ct-firmware
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT)
endef

QCA988X_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-19.bin.lede
define Download/ath10k-firmware-qca988x-ct
  $(call Download/ct-firmware,QCA988X,)
  HASH:=bff98f028062dae9fc638c7596aec3c79bf9eddaff65cfacba067f6d72f217cd
endef
$(eval $(call Download,ath10k-firmware-qca988x-ct))

QCA9887_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-19.bin.lede
define Download/ath10k-firmware-qca9887-ct
  $(call Download/ct-firmware,QCA9887,ath10k-9887)
  HASH:=95dc106f98672bd9c7d3fe6881ed79ab079cb49b0a995650991b1beaff2b0101
endef
$(eval $(call Download,ath10k-firmware-qca9887-ct))

QCA99X0_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-9.bin-lede.004
define Download/ath10k-firmware-qca99x0-ct
  $(call Download/ct-firmware,QCA99X0,ath10k-10-4)
  HASH:=993c29fd64bb2a59b86d34f58601a1a48b83b541750bc511f78cc17152829b4d
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct))

QCA9984_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-9.bin-lede.004
define Download/ath10k-firmware-qca9984-ct
  $(call Download/ct-firmware,QCA9984,ath10k-9984-10-4)
  HASH:=d997eed9a8bc6809c01d367759ba8545c10e3be93ea1f33d6d753127ef0f7c5e
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct))

QCA9888_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-9.bin-lede.004
define Download/ath10k-firmware-qca9888-ct
  $(call Download/ct-firmware,QCA9888,ath10k-9888-10-4)
  HASH:=bbaa71bc7dcaa264c5875e86639f174908fed09fbace975e325959d42f3754ff
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct))

define Package/ath10k-firmware-qca99x0
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA99x0 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca988x-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.1 firmware for QCA988x devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca9887-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.1 firmware for QCA9887 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca988x-ct/description
Alternative ath10k firmware for QCA988X from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware will NOT be used unless the standard ath10k-firmware-qca988x
is un-selected since the driver will try to load firmware-5.bin before
firmware-2.bin
endef

define Package/ath10k-firmware-qca9887-ct/description
Alternative ath10k firmware for QCA9887 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware conflicts with the standard 9887 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca99x0-ct/description
Alternative ath10k firmware for QCA99x0 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 99x0 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca9984-ct/description
Alternative ath10k firmware for QCA9984 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 9984 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca9888-ct/description
Alternative ath10k firmware for QCA9886 and QCA9888 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 9886 and 9888 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca99x0/description
Standard ath10k firmware for QCA99x0 from QCA
This firmware conflicts with the CT 99x0 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca99x0-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4.3 firmware for QCA99x0 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca9984-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4.3 firmware for QCA9984 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca9888-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4.3 firmware for QCA9886 and QCA9888 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca9984
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA9984 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca4019
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for IPQ/QCA4019 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Package/ath10k-firmware-qca6174
$(Package/ath10k-firmware-default)
  TITLE:=ath10k firmware for QCA6174 devices
  SECTION:=firmware
  CATEGORY:=Firmware
endef

QCA99X0_BOARD_REV:=ddcec9efd245da9365c474f513a855a55f3ac7fe
QCA99X0_BOARD_FILE:=board-2.bin.$(QCA99X0_BOARD_REV)

define Download/qca99x0-board
  URL:=https://source.codeaurora.org/quic/qsdk/oss/firmware/ath10k-firmware/plain/ath10k/QCA99X0/hw2.0
  URL_FILE:=board-2.bin?id=$(QCA99X0_BOARD_REV)
  FILE:=$(QCA99X0_BOARD_FILE)
  HASH:=03711ac21e60ef59d3815e235eb721c0c22851b5410299411085aa6f2af45401
endef
$(eval $(call Download,qca99x0-board))

define Build/Compile

endef

define Package/ath10k-firmware-qca4019/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA4019/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA4019/hw1.0/3.2.1/firmware-5.bin_10.4-3.2.1-00053 \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9887/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9887/hw1.0/10.2.4-1.0/firmware-5.bin_10.2.4-1.0-00029 \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-5.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9887/hw1.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef

define Package/ath10k-firmware-qca9888/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9888/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9888/hw2.0/firmware-5.bin_10.4-3.2-00072 \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca988x/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/10.2.4-1.0/firmware-5.bin_10.2.4-1.0-00029 \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9887-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9887) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9887/hw1.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef

define Package/ath10k-firmware-qca988x-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA988X) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-2.bin
endef

define Package/ath10k-firmware-qca6174/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k
	$(CP) $(PKG_BUILD_DIR)/QCA6174 $(1)/lib/firmware/ath10k/
endef

define Package/ath10k-firmware-qca99x0/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA99X0/hw2.0/boardData_AR900B_CUS239_5G_v2_001.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA99X0/hw2.0/firmware-5.bin_10.4.1.00030-1 \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca99x0-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA99X0/hw2.0/boardData_AR900B_CUS239_5G_v2_001.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA99X0) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9984/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/3.4/firmware-5.bin_10.4-3.4-00082 \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9984-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9984) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9888-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	ln -s \
		../../cal-pci-0000:01:00.0.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9888/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9888) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef

$(eval $(call BuildPackage,ath10k-firmware-qca9887))
$(eval $(call BuildPackage,ath10k-firmware-qca9888))
$(eval $(call BuildPackage,ath10k-firmware-qca988x))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0))
$(eval $(call BuildPackage,ath10k-firmware-qca6174))
$(eval $(call BuildPackage,ath10k-firmware-qca9984))
$(eval $(call BuildPackage,ath10k-firmware-qca4019))

$(eval $(call BuildPackage,ath10k-firmware-qca9887-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca988x-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct))
