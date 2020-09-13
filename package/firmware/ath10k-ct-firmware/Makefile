include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-ct-firmware
PKG_VERSION:=2020-07-02
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

ATH10K_FIRMWARE_REV:=d622d160e9f552ead68d9ae81b715422892dc2ef
ATH10K_FIRMWARE_URL:=@GITHUB/kvalo/ath10k-firmware/$(ATH10K_FIRMWARE_REV)

QCA9887_BOARD_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca9887-board.bin
define Download/qca9887-board
  FILE:=$(QCA9887_BOARD_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA9887/hw1.0
  URL_FILE:=board.bin
  HASH:=cf4df099f6ee05c181f55ce17297a1d32c61d725eb96246fd315ad5587c42426
endef
$(eval $(call Download,qca9887-board))

QCA988X_BOARD_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca988x-board.bin
define Download/qca988x-board
  FILE:=$(QCA988X_BOARD_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA988X/hw2.0
  URL_FILE:=board.bin
  HASH:=5b5b380333c2dd3b6ce67f30e2f7008f4020bf594970d3b464fd8d4a80fcd880
endef
$(eval $(call Download,qca988x-board))

QCA99X0_BOARD_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca99x0-board.bin
define Download/qca99x0-board
  FILE:=$(QCA99X0_BOARD_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA99X0/hw2.0
  URL_FILE:=boardData_AR900B_CUS239_5G_v2_001.bin
  HASH:=3bf7561ee373b369025dcd366d276d038a97d3397ccae41ce841d98a58b30aff
endef
$(eval $(call Download,qca99x0-board))

QCA99X0_BOARD2_REV:=ddcec9efd245da9365c474f513a855a55f3ac7fe
QCA99X0_BOARD2_FILE:=ath10k-firmware-$(QCA99X0_BOARD2_REV)-qca99x0-board-2.bin
define Download/qca99x0-board2
  FILE:=$(QCA99X0_BOARD2_FILE)
  URL:=https://source.codeaurora.org/quic/qsdk/oss/firmware/ath10k-firmware/plain/ath10k/QCA99X0/hw2.0
  URL_FILE:=board-2.bin?id=$(QCA99X0_BOARD2_REV)
  HASH:=03711ac21e60ef59d3815e235eb721c0c22851b5410299411085aa6f2af45401
endef
$(eval $(call Download,qca99x0-board2))

QCA9984_BOARD2_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca9984-board-2.bin
define Download/qca9984-board2
  FILE:=$(QCA9984_BOARD2_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA9984/hw1.0
  URL_FILE:=board-2.bin
  HASH:=0d6d46cf0467185e3959ce3cb69e2415be6e48ab8a4bee3eb400edbe48cb9c25
endef
$(eval $(call Download,qca9984-board2))

QCA4019_BOARD2_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca4019-board-2.bin
define Download/qca4019-board2
  FILE:=$(QCA4019_BOARD2_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA4019/hw1.0
  URL_FILE:=board-2.bin
  HASH:=94b66aa4ddbed5110a96364d3c7b4ebcb320e3ac4e8697660b277e76077bc338
endef
$(eval $(call Download,qca4019-board2))

QCA9888_BOARD2_FILE:=ath10k-firmware-$(ATH10K_FIRMWARE_REV)-qca9888-board-2.bin
define Download/qca9888-board2
  FILE:=$(QCA9888_BOARD2_FILE)
  URL:=$(ATH10K_FIRMWARE_URL)/QCA9888/hw2.0
  URL_FILE:=board-2.bin
  HASH:=5b871bb567f64525ca45adb88063211de472015d09e0f9aa3fa61ab71c8fdfd3
endef
$(eval $(call Download,qca9888-board2))

CT_FIRMWARE_FILE = $(1)-$($(1)_FIRMWARE_FILE_CT)
CT_FIRMWARE_FILE_FULL_HTT = $(1)-$($(1)_FIRMWARE_FILE_CT_FULL_HTT)
CT_FIRMWARE_FILE_HTT = $(1)-$($(1)_FIRMWARE_FILE_CT_HTT)

define Download/ct-firmware
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT)
endef

define Download/ct-firmware-full-htt
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE_FULL_HTT,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT_FULL_HTT)
endef

define Download/ct-firmware-htt
  URL:=https://www.candelatech.com/downloads/$(2)
  FILE:=$(call CT_FIRMWARE_FILE_HTT,$(1))
  URL_FILE:=$($(1)_FIRMWARE_FILE_CT_HTT)
endef

QCA988X_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-22.bin.lede.019
define Download/ath10k-firmware-qca988x-ct
  $(call Download/ct-firmware,QCA988X,)
  HASH:=8b4c99253aa309d35f2e060c190091b8db1b84dbda06a6a15c83ac0f9a938126
endef
$(eval $(call Download,ath10k-firmware-qca988x-ct))

QCA988X_FIRMWARE_FILE_CT_FULL_HTT:=firmware-2-ct-full-htt-mgt-community-22.bin.lede.019
define Download/ath10k-firmware-qca988x-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA988X,)
  HASH:=a7168916d6aa5e4d7858f8b620c0c980c76d03f390929db6f4077685ce2051e7
endef
$(eval $(call Download,ath10k-firmware-qca988x-ct-full-htt))


QCA9887_FIRMWARE_FILE_CT:=firmware-2-ct-full-community-22.bin.lede.019
define Download/ath10k-firmware-qca9887-ct
  $(call Download/ct-firmware,QCA9887,ath10k-9887)
  HASH:=459692deb186a63ab8eeddb7ad5d54779266e68ca686e7c46062554db6dca12b
endef
$(eval $(call Download,ath10k-firmware-qca9887-ct))

QCA9887_FIRMWARE_FILE_CT_FULL_HTT:=firmware-2-ct-full-htt-mgt-community-22.bin.lede.019
define Download/ath10k-firmware-qca9887-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA9887,ath10k-9887)
  HASH:=fd126a457d0927d0c8ea10d66ef5b67d5e1e0741f8692bb3016bb602d0af3098
endef
$(eval $(call Download,ath10k-firmware-qca9887-ct-full-htt))


QCA99X0_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-12.bin-lede.019
define Download/ath10k-firmware-qca99x0-ct
  $(call Download/ct-firmware,QCA99X0,ath10k-10-4b)
  HASH:=7dc934f934bc4973c9273a4f22cfead8e26ec6f579647af31b718a860eca0a4b
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct))

QCA99X0_FIRMWARE_FILE_CT_FULL_HTT:=firmware-5-ct-full-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca99x0-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA99X0,ath10k-10-4b)
  HASH:=71a27b245a382fe009938d2826d5c97a90dceb10ddf638325268df91837ea302
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct-full-htt))

QCA99X0_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca99x0-ct-htt
  $(call Download/ct-firmware-htt,QCA99X0,ath10k-10-4b)
  HASH:=9ed4fe41e5b0f30172f71ae0fe382dc0aab8aa4f8a898417af4f7ee936575ef6
endef
$(eval $(call Download,ath10k-firmware-qca99x0-ct-htt))


QCA9984_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-12.bin-lede.019
define Download/ath10k-firmware-qca9984-ct
  $(call Download/ct-firmware,QCA9984,ath10k-9984-10-4b)
  HASH:=32d13f432691fe759ded7d027052e925233adb436cd8f729f85ec3d19ccd1dfd
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct))

QCA9984_FIRMWARE_FILE_CT_FULL_HTT:=firmware-5-ct-full-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca9984-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA9984,ath10k-9984-10-4b)
  HASH:=e8ab69777bd00b5fc6b1b7acccb55b903553a99932a5b0351602b5f690106588
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct-full-htt))

QCA9984_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca9984-ct-htt
  $(call Download/ct-firmware-htt,QCA9984,ath10k-9984-10-4b)
  HASH:=74449b303b626e0713b3fd4f2d6103d65859403b2dd7bdd8882aa772b69b59c7
endef
$(eval $(call Download,ath10k-firmware-qca9984-ct-htt))


QCA4019_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-12.bin-lede.019
define Download/ath10k-firmware-qca4019-ct
  $(call Download/ct-firmware,QCA4019,ath10k-4019-10-4b)
  HASH:=4b89763087c7ed9b56046c4e621b7f045e452436d8d9b430a5d171179e313592
endef
$(eval $(call Download,ath10k-firmware-qca4019-ct))

QCA4019_FIRMWARE_FILE_CT_FULL_HTT:=firmware-5-ct-full-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca4019-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA4019,ath10k-4019-10-4b)
  HASH:=fba591e5777c53b82542ba16cae69d9bb4684837f2fa4cee1b9b26f648096748
endef
$(eval $(call Download,ath10k-firmware-qca4019-ct-full-htt))

QCA4019_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca4019-ct-htt
  $(call Download/ct-firmware-htt,QCA4019,ath10k-4019-10-4b)
  HASH:=0d534c3c424184b8ec2773f15c8933bdab0d39b6f664d2578c6602b0eb7035d1
endef
$(eval $(call Download,ath10k-firmware-qca4019-ct-htt))


QCA9888_FIRMWARE_FILE_CT:=firmware-5-ct-full-community-12.bin-lede.019
define Download/ath10k-firmware-qca9888-ct
  $(call Download/ct-firmware,QCA9888,ath10k-9888-10-4b)
  HASH:=048f4300725e6ebbf94a6bf4f3f4e4592c446fcdbe1d801aaac024b15e89e0c9
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct))

QCA9888_FIRMWARE_FILE_CT_FULL_HTT:=firmware-5-ct-full-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca9888-ct-full-htt
  $(call Download/ct-firmware-full-htt,QCA9888,ath10k-9888-10-4b)
  HASH:=d2a7e9fea6bd854721b3fc03a3a00d379d303b2bce339377ee87a1c14a60312d
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct-full-htt))

QCA9888_FIRMWARE_FILE_CT_HTT:=firmware-5-ct-htt-mgt-community-12.bin-lede.019
define Download/ath10k-firmware-qca9888-ct-htt
  $(call Download/ct-firmware-htt,QCA9888,ath10k-9888-10-4b)
  HASH:=e52a6db33347c641ee791fd9a3a57a2503cdda1adc6b8d943e336431528b9d2a
endef
$(eval $(call Download,ath10k-firmware-qca9888-ct-htt))


define Package/ath10k-ct-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://www.candelatech.com/ath10k.php
  DEPENDS:=
endef

define Package/ath10k-firmware-qca988x-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.1 firmware for QCA988x devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca988x
endef
define Package/ath10k-firmware-qca988x-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.1 full-htt-mgt fw for QCA988x
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca988x
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9887-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.1 firmware for QCA9887 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9887
endef
define Package/ath10k-firmware-qca9887-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.1 full-htt-mgt fw for QCA9887
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9887
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca99x0-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA99x0 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca99x0
endef
define Package/ath10k-firmware-qca99x0-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 full-htt-mgt fw for QCA99x0
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca99x0
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef
define Package/ath10k-firmware-qca99x0-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA99x0
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca99x0
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9984-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA9984 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9984
endef
define Package/ath10k-firmware-qca9984-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 full-htt-mgt fw for QCA9984
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9984
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef
define Package/ath10k-firmware-qca9984-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA9984
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9984
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca4019-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 firmware for QCA4018/9
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca4019
endef
define Package/ath10k-firmware-qca4019-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 full-htt-mgt for QCA4018/9
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca4019
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef
define Package/ath10k-firmware-qca4019-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt for QCA4018/9
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca4019
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef

define Package/ath10k-firmware-qca9888-ct
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 fw for QCA9886/8 devices
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9888
endef
define Package/ath10k-firmware-qca9888-ct-full-htt
$(Package/ath10k-ct-firmware-default)
  TITLE:=ath10k CT 10.4 full-htt-mgt fw for QCA9886/8
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9888
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef
define Package/ath10k-firmware-qca9888-ct-htt
$(Package/ath10k-firmware-default)
  TITLE:=ath10k CT 10.4 htt-mgt fw for QCA9886/8
  SECTION:=firmware
  CATEGORY:=Firmware
  PROVIDES:=ath10k-firmware-qca9888
  DEPENDS:=+!PACKAGE_kmod-ath10k-ct-smallbuffers:kmod-ath10k-ct
endef


define Package/ath10k-firmware-qca9887-ct/description
Alternative ath10k firmware for QCA9887 from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware conflicts with the standard 9887 firmware, so select only
one.
endef
define Package/ath10k-firmware-qca9887-ct-full-htt/description
Alternative ath10k firmware for QCA9887 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and fixes .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca988x-ct/description
Alternative ath10k firmware for QCA988X from Candela Technologies.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.1.php
This firmware will NOT be used unless the standard ath10k-firmware-qca988x
is un-selected since the driver will try to load firmware-5.bin before
firmware-2.bin
endef
define Package/ath10k-firmware-qca988x-ct-full-htt/description
Alternative ath10k firmware for QCA988X from Candela Technologies.
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
define Package/ath10k-firmware-qca99x0-ct-full-htt/description
Alternative ath10k firmware for QCA99x0 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef
define Package/ath10k-firmware-qca99x0-ct-htt/description
Alternative ath10k firmware for QCA99x0 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
This firmware lacks a lot of features that ath10k does not use, saving
a lot of resources.
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
define Package/ath10k-firmware-qca9984-ct-full-htt/description
Alternative ath10k firmware for QCA9984 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef
define Package/ath10k-firmware-qca9984-ct-htt/description
Alternative ath10k firmware for QCA9984 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
This firmware lacks a lot of features that ath10k does not use, saving
a lot of resources.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef

define Package/ath10k-firmware-qca4019-ct/description
Alternative ath10k firmware for IPQ4019 radio from Candela Technologies.
Enables IBSS and other features.  Works with standard or ath10k-ct driver.
See:  http://www.candelatech.com/ath10k-10.4.php
endef
define Package/ath10k-firmware-qca4019-ct-full-htt/description
Alternative ath10k firmware for IPQ4019 radio from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.
See:  http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef
define Package/ath10k-firmware-qca4019-ct-htt/description
Alternative ath10k firmware for IPQ4019 radio from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
This firmware lacks a lot of features that ath10k does not use, saving
a lot of resources.
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
define Package/ath10k-firmware-qca9888-ct-full-htt/description
Alternative ath10k firmware for QCA9886 and QCA9888 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef
define Package/ath10k-firmware-qca9888-ct-htt/description
Alternative ath10k firmware for QCA9886 and QCA9888 from Candela Technologies.
Uses normal HTT TX data path for management frames, which improves
stability in busy networks and may be required for .11r authentication.
This firmware lacks a lot of features that ath10k does not use, saving
a lot of resources.
Enables IBSS and other features.  See:
http://www.candelatech.com/ath10k-10.4.php
This firmware selects and requires the ath10k-ct driver.
endef


define Build/Compile

endef


define Package/ath10k-firmware-qca9887-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9887) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9887_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef
define Package/ath10k-firmware-qca9887-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA9887) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/ct-firmware-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9887_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef

define Package/ath10k-firmware-qca988x-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA988X_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA988X) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-2.bin
endef
define Package/ath10k-firmware-qca988x-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA988X_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA988X) \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/ct-firmware-2.bin
endef

define Package/ath10k-firmware-qca99x0-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA99X0) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/firmware-5.bin
endef
define Package/ath10k-firmware-qca99x0-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA99X0) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/ct-firmware-5.bin
endef
define Package/ath10k-firmware-qca99x0-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA99X0_BOARD_FILE) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA99X0) \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/ct-firmware-5.bin
endef

define Package/ath10k-firmware-qca9984-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9984_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9984) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef
define Package/ath10k-firmware-qca9984-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9984_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA9984) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/ct-firmware-5.bin
endef
define Package/ath10k-firmware-qca9984-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9984_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA9984) \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/ct-firmware-5.bin
endef

define Package/ath10k-firmware-qca4019-ct/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA4019_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA4019) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef
define Package/ath10k-firmware-qca4019-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA4019_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA4019) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/ct-firmware-5.bin
endef
define Package/ath10k-firmware-qca4019-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA4019_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/board-2.bin
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
		$(DL_DIR)/$(QCA9888_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE,QCA9888) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef
define Package/ath10k-firmware-qca9888-ct-full-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	ln -s \
		../../cal-pci-0000:01:00.0.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9888_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_FULL_HTT,QCA9888) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/ct-firmware-5.bin
endef
define Package/ath10k-firmware-qca9888-ct-htt/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	ln -s \
		../../cal-pci-0000:01:00.0.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(QCA9888_BOARD2_FILE) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(DL_DIR)/$(call CT_FIRMWARE_FILE_HTT,QCA9888) \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/ct-firmware-5.bin
endef


$(eval $(call BuildPackage,ath10k-firmware-qca9887-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9887-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca988x-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca988x-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca99x0-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9984-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca4019-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca4019-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca4019-ct-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct-full-htt))
$(eval $(call BuildPackage,ath10k-firmware-qca9888-ct-htt))
