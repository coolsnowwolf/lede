#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-firmware
PKG_SOURCE_DATE:=2018-02-09
PKG_SOURCE_VERSION:=8f4bafdd400d21a65966004d0ce6e0686ef4d9bc
PKG_MIRROR_HASH:=4f4f0678b9d07c0282f18c69bd63a5e2a2ae015b9ce7298cedb88a60be87ed3a
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
CT_FIRMWARE_FILE_HTT = $(1)-$($(1)_FIRMWARE_FILE_CT_HTT)

define Download/ct-firmware
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT)
endef

define Download/ct-firmware-htt
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE_HTT,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT_HTT)
endef

QCA988X_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-20.bin.lede.006
define Download/ath10k-firmware-qca988x-ct
  $(call Download/ct-firmware,QCA988X,)
  HASH:=b28ff3eb10f15033be68bbedc91abcf40f7369cc44da4273e3c0df285c74a4f8
endef
$(eval $(call Download,ath10k-firmware-qca988x-ct))

QCA988X_FIRMWARE_FILE_CT_HTT:=firmware-2-ct-full-htt-mgt-community-20.bin.lede.006
define Download/ath10k-firmware-qca988x-ct-htt
  $(call Download/ct-firmware-htt,QCA988X,)
  HASH:=08dd8d339079c9a704cf3f91787cc06a73168a9f57eeb777edaa9cd4194c246c
endef
$(eval $(call Download,ath10k-firmware-qca988x-ct-htt))


QCA9887_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-20.bin.lede.006
define Download/ath10k-firmware-qca9887-ct
  $(call Download/ct-firmware,QCA9887,ath10k-9887)
  HASH:=8d3fd2fb62479efa955e0d0487ca8971409e68b395f3a2df28569ec243bd937d
endef
$(eval $(call Download,ath10k-firmware-qca9887-ct))

QCA9887_FIRMWARE_FILE_CT_HTT:=firmware-2-ct-full-htt-mgt-community-20.bin.lede.006
define Download/ath10k-firmware-qca9887-ct-htt
  $(call Download/ct-firmware-htt,QCA9887,ath10k-9887)
  HASH:=f192b5a8ac4b90d828cace0a2279c02fda689fa936f5154f08df6658d0230e9b
endef
$(eval $(call Download,ath10k-firmware-qca9887-ct-htt))


QCA99X0_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-10.bin-lede.006
define Download/ath10k-firmware-qca99x0-ct
  $(call Download/ct-firmware,QCA99X0,ath10k-10-4)
  HASH:=1aa21acab7974651af6c0a2698891fc8792edb5124824fac1587123d3b48df67
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct))

QCA99X0_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-full-htt-mgt-community-10.bin-lede.006
define Download/ath10k-firmware-qca99x0-ct-htt
  $(call Download/ct-firmware-htt,QCA99X0,ath10k-10-4)
  HASH:=e3d89c00f194975649b2f41ff5f5ffc34d60c393d15800df6ea56a8ac29aeeee
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct-htt))


QCA9984_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-10.bin-lede.006
define Download/ath10k-firmware-qca9984-ct
  $(call Download/ct-firmware,QCA9984,ath10k-9984-10-4)
  HASH:=f9700cafb4e2cebe8364eb6fe1754eb2c37b1b02a7408ded908ecc78954380a3
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct))

QCA9984_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-full-htt-mgt-community-10.bin-lede.006
define Download/ath10k-firmware-qca9984-ct-htt
  $(call Download/ct-firmware-htt,QCA9984,ath10k-9984-10-4)
  HASH:=c8c288cd38f3e6b378d7ed8407ee6b01cfcc73acc03d6ad5b8d392a8a346dd82
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct-htt))


QCA4019_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-10.bin-lede.006
define Download/ath10k-firmware-qca4019-ct
  $(call Download/ct-firmware,QCA4019,ath10k-4019-10-4)
  HASH:=d2207982b49959b9526d4dc1fd4181b49b3d43a841cf5d6cc396ccb1bc4b572c
endef
$(eval $(call Download,ath10k-firmware-qca4019-ct))

QCA4019_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-full-htt-mgt-community-10.bin-lede.006
define Download/ath10k-firmware-qca4019-ct-htt
  $(call Download/ct-firmware-htt,QCA4019,ath10k-4019-10-4)
  HASH:=3c900bb656f093fe6882616d788c6e2e8fc09edbdda06691e16e5f08d79c4955
endef
$(eval $(call Download,ath10k-firmware-qca4019-ct-htt))


QCA9888_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-10.bin-lede.006
define Download/ath10k-firmware-qca9888-ct
  $(call Download/ct-firmware,QCA9888,ath10k-9888-10-4)
  HASH:=9bd38e80ec52484bae5edc785df09cf9acdad3e28ee5080a8105ba786118193a
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct))

QCA9888_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-full-htt-mgt-community-10.bin-lede.006
define Download/ath10k-firmware-qca9888-ct-htt
  $(call Download/ct-firmware-htt,QCA9888,ath10k-9888-10-4)
  HASH:=63e695f55400f784ca8123c299e240626a403437b30d197a7e69a4b42d155b8a
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct-htt))


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
  PROVIDES:=ath10k-firmware-qca988x
endef
define Package/ath10k-firmware-qca988x-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.1 htt-mgt fw for QCA988x
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca988x
  DEPENDS:=+kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9887-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.1 firmware for QCA9887 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9887
endef
define Package/ath10k-firmware-qca9887-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.1 htt-mgt fw for QCA9887
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9887
  DEPENDS:=+kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca988x-ct/description
Alternative ath10k firmware for QCA988X from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware will NOT be used unless the standard ath10k-firmware-qca988x
is un-selected since the driver will try to load firmware-5.bin before
firmware-2.bin
endef
define Package/ath10k-firmware-qca988x-ct-htt/description
Alternative ath10k firmware for QCA988X from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and fixes .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca9887-ct/description
Alternative ath10k firmware for QCA9887 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware conflicts with the standard 9887 firmware, so select only
one.
endef
define Package/ath10k-firmware-qca9887-ct-htt/description
Alternative ath10k firmware for QCA9887 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and fixes .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca99x0-ct/description
Alternative ath10k firmware for QCA99x0 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 99x0 firmware, so select only
one.
endef
define Package/ath10k-firmware-qca99x0-ct-htt/description
Alternative ath10k firmware for QCA99x0 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca9984-ct/description
Alternative ath10k firmware for QCA9984 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 9984 firmware, so select only
one.
endef
define Package/ath10k-firmware-qca9984-ct-htt/description
Alternative ath10k firmware for QCA9984 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca4019-ct/description
Alternative ath10k firmware for IPQ4019 radio from Candela Technologies.
Enables IBSS and other features.  Works with standard or ath10k-ct driver.
See:  http://www.candelatech.com/ath10k-10.4.php
endef
define Package/ath10k-firmware-qca4019-ct-htt/description
Alternative ath10k firmware for IPQ4019 radio from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.
See:  http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca9888-ct/description
Alternative ath10k firmware for QCA9886 and QCA9888 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware conflicts with the standard 9886 and 9888 firmware, so select only
one.
endef
define Package/ath10k-firmware-qca9888-ct-htt/description
Alternative ath10k firmware for QCA9886 and QCA9888 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef


define Package/ath10k-firmware-qca99x0/description
Standard ath10k firmware for QCA99x0 from QCA
This firmware conflicts with the CT 99x0 firmware, so select only
one.
endef

define Package/ath10k-firmware-qca99x0-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA99x0 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca99x0
endef
define Package/ath10k-firmware-qca99x0-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA99x0
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca99x0
  DEPENDS:=+kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9984-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA9984 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9984
endef
define Package/ath10k-firmware-qca9984-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA9984
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9984
  DEPENDS:=+kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca4019-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA4018/9
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca4019
endef
define Package/ath10k-firmware-qca4019-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt for QCA4018/9
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca4019
  DEPENDS:=+kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9888-ct
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 fw for QCA9886/8 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9888
endef
define Package/ath10k-firmware-qca9888-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA9886/8
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9888
  DEPENDS:=+kmod-ath10k-ct
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
		$(PKG_BUILD_DIR)/QCA4019/hw1.0/3.4/firmware-5.bin_10.4-3.4-00104 \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca9887/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9887/hw1.0/10.2.4-1.0/firmware-5.bin_10.2.4-1.0-00033 \
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
		$(PKG_BUILD_DIR)/QCA9888/hw2.0/3.5.3/firmware-5.bin_10.4-3.5.3-00053 \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef

define Package/ath10k-firmware-qca988x/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/10.2.4-1.0/firmware-5.bin_10.2.4-1.0-00033 \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-5.bin
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

define Package/ath10k-firmware-qca9984/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/3.5.3/firmware-5.bin_10.4-3.5.3-00053 \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef


# CT related firmware

define Package/ath10k-firmware-qca9887-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9887) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9887/hw1.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef
define Package/ath10k-firmware-qca9887-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA9887) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/ct-firmware-2.bin
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
define Package/ath10k-firmware-qca988x-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA988X) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/ct-firmware-2.bin
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
define Package/ath10k-firmware-qca99x0-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA99X0/hw2.0/boardData_AR900B_CUS239_5G_v2_001.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA99X0) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/ct-firmware-5.bin
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
define Package/ath10k-firmware-qca9984-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA9984) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/ct-firmware-5.bin
endef

define Package/ath10k-firmware-qca4019-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA4019/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA4019) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef
define Package/ath10k-firmware-qca4019-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA4019/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA4019) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/ct-firmware-5.bin
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
define Package/ath10k-firmware-qca9888-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	ln -s \
		../../cal-pci-0000:01:00.0.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/QCA9888/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA9888) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/ct-firmware-5.bin
endef

$(eval $(call BuildPackage,ath10k-firmware-qca9887))
$(eval $(call BuildPackage,ath10k-firmware-qca9888))
$(eval $(call BuildPackage,ath10k-firmware-qca988x))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0))
$(eval $(call BuildPackage,ath10k-firmware-qca6174))
$(eval $(call BuildPackage,ath10k-firmware-qca9984))
$(eval $(call BuildPackage,ath10k-firmware-qca4019))

$(eval $(call BuildPackage,ath10k-firmware-qca9887-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9887-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca988x-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca988x-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca4019-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca4019-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct-htt))
