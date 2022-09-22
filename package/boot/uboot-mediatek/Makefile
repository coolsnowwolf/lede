include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2022.07
PKG_HASH:=92b08eb49c24da14c1adbf70a71ae8f37cc53eeb4230e859ad8b6733d13dcf5e
PKG_BUILD_DEPENDS:=arm-trusted-firmware-tools/host

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

MT7621_LOWLEVEL_PRELOADER_URL:=https://raw.githubusercontent.com/mtk-openwrt/mt7621-lowlevel-preloader/a03b07c60bf1ba4add9b671d32caa102fe948180/

define Download/mt7621-stage-sram
  FILE:=mt7621_stage_sram.bin
  URL:=$(MT7621_LOWLEVEL_PRELOADER_URL)
  HASH:=1dda68aa089f0ff262e01539b990dea478952e9fb68bcc0a8cd6f76f0135c62e
endef

define Download/mt7621-stage-sram-noprint
  FILE:=mt7621_stage_sram_noprint.bin
  URL:=$(MT7621_LOWLEVEL_PRELOADER_URL)
  HASH:=8ee419275144fc298e9444d413d98e965a55d283152a74ea6a1f8de79eb516b6
endef

ifdef CONFIG_TARGET_ramips_mt7621
ifdef CONFIG_DEBUG
$(eval $(call Download,mt7621-stage-sram))
else
$(eval $(call Download,mt7621-stage-sram-noprint))
endif
endif

define U-Boot/Default
  BUILD_TARGET:=mediatek
  UBOOT_IMAGE:=u-boot-mtk.bin
endef

define U-Boot/mt7620_rfb
  NAME:=MT7620 Reference Board
  UBOOT_CONFIG:=mt7620_rfb
  BUILD_DEVICES:=ralink_mt7620a-evb
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7620
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/mt7620_mt7530_rfb
  NAME:=MT7620+MT7530 Reference Board
  UBOOT_CONFIG:=mt7620_mt7530_rfb
  BUILD_DEVICES:=ralink_mt7620a-mt7530-evb
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7620
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/mt7621_rfb
  NAME:=MT7621 Reference Board
  UBOOT_CONFIG:=mt7621_rfb
  BUILD_DEVICES:=mediatek_mt7621-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7621
  UBOOT_IMAGE:=u-boot-mt7621.bin
endef

define U-Boot/mt7621_nand_rfb
  NAME:=MT7621 Reference Board (NAND)
  UBOOT_CONFIG:=mt7621_nand_rfb
  BUILD_DEVICES:=mediatek_mt7621-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7621
  UBOOT_IMAGE:=u-boot-mt7621.bin
endef

define U-Boot/mt7622_rfb1
  NAME:=MT7622 Reference Board 1
  UBOOT_CONFIG:=mt7622_rfb
  BUILD_DEVICES:=mediatek_mt7622-rfb1 mediatek_mt7622-rfb1-ubi
  BUILD_SUBTARGET:=mt7622
endef

define U-Boot/mt7622_linksys_e8450
  NAME:=Linksys E8450
  UBOOT_CONFIG:=mt7622_linksys_e8450
  BUILD_DEVICES:=linksys_e8450-ubi
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand
  BL2_DDRBLOB:=1
  DEPENDS:=+trusted-firmware-a-mt7622-snand-1ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-emmc
  NAME:=BananaPi R64 (eMMC)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-emmc
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-emmc-2ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-sdmmc
  NAME:=BananaPi R64 (SDMMC)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-sdmmc
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-sdmmc-2ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-snand
  NAME:=BananaPi R64 (SNAND)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-snand
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-snand-2ddr
endef

define U-Boot/mt7622_ubnt_unifi-6-lr
  NAME:=Ubiquiti UniFi 6 LR
  UBOOT_CONFIG:=mt7622_ubnt_unifi-6-lr
  BUILD_DEVICES:=ubnt_unifi-6-lr-v1-ubootmod ubnt_unifi-6-lr-v2-ubootmod
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-nor-2ddr
  FIP_COMPRESS:=1
endef

define U-Boot/mt7623a_unielec_u7623
  NAME:=UniElec U7623 (mt7623)
  BUILD_DEVICES:=unielec_u7623-02
  BUILD_SUBTARGET:=mt7623
  UBOOT_CONFIG:=mt7623a_unielec_u7623_02
endef

define U-Boot/mt7623n_bpir2
  NAME:=Banana Pi R2 (mt7623)
  BUILD_DEVICES:=bananapi_bpi-r2
  BUILD_SUBTARGET:=mt7623
  UBOOT_IMAGE:=u-boot.bin
  UBOOT_CONFIG:=mt7623n_bpir2
endef

define U-Boot/mt7628_rfb
  NAME:=MT7628 Reference Board
  BUILD_DEVICES:=mediatek_mt7628an-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt76x8
  UBOOT_CONFIG:=mt7628_rfb
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/ravpower_rp-wd009
  NAME:=RAVPower RP-WD009
  BUILD_TARGET:=ramips
  BUILD_DEVICES:=ravpower_rp-wd009
  BUILD_SUBTARGET:=mt76x8
  UBOOT_CONFIG:=ravpower-rp-wd009-ram
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/mt7629_rfb
  NAME:=MT7629 Reference Board
  BUILD_SUBTARGET:=mt7629
  BUILD_DEVICES:=mediatek_mt7629-rfb
  UBOOT_CONFIG:=mt7629_rfb
endef

define U-Boot/mt7986_rfb
  NAME:=MT7986 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7986-rfb
  UBOOT_CONFIG:=mt7986_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-sdmmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-emmc
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-emmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-sdmmc
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-sd
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-sdmmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-snand
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-snand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-nor
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-nor
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-nor-ddr4
  FIP_COMPRESS:=1
endef

UBOOT_TARGETS := \
	mt7620_mt7530_rfb \
	mt7620_rfb \
	mt7621_nand_rfb \
	mt7621_rfb \
	mt7622_bananapi_bpi-r64-emmc \
	mt7622_bananapi_bpi-r64-sdmmc \
	mt7622_bananapi_bpi-r64-snand \
	mt7622_linksys_e8450 \
	mt7622_rfb1 \
	mt7622_ubnt_unifi-6-lr \
	mt7623n_bpir2 \
	mt7623a_unielec_u7623 \
	mt7628_rfb \
	ravpower_rp-wd009 \
	mt7629_rfb \
	mt7986_bananapi_bpi-r3-emmc \
	mt7986_bananapi_bpi-r3-sdmmc \
	mt7986_bananapi_bpi-r3-snand \
	mt7986_bananapi_bpi-r3-nor \
	mt7986_rfb

ifdef CONFIG_TARGET_mediatek
UBOOT_MAKE_FLAGS += $(UBOOT_IMAGE:.fip=.bin)
endif

define Build/fip-image
	$(if $(FIP_COMPRESS),\
		xz -f -e -k -9 -C crc32 $(STAGING_DIR_IMAGE)/$(if $(BL2_SOC),$(BL2_SOC),$(BUILD_SUBTARGET))-$(BL2_BOOTDEV)-$(if $(BL2_DDRTYPE),$(BL2_DDRTYPE)-)$(if $(BL2_DDRBLOB),$(BL2_DDRBLOB)ddr-)bl31.bin ;\
		xz -f -e -k -9 -C crc32 $(PKG_BUILD_DIR)/u-boot.bin \
	)
	$(STAGING_DIR_HOST)/bin/fiptool create \
		--soc-fw $(STAGING_DIR_IMAGE)/$(if $(BL2_SOC),$(BL2_SOC),$(BUILD_SUBTARGET))-$(BL2_BOOTDEV)-$(if $(BL2_DDRTYPE),$(BL2_DDRTYPE)-)$(if $(BL2_DDRBLOB),$(BL2_DDRBLOB)ddr-)bl31.bin$(if $(FIP_COMPRESS),.xz) \
		--nt-fw $(PKG_BUILD_DIR)/u-boot.bin$(if $(FIP_COMPRESS),.xz) \
		$(PKG_BUILD_DIR)/u-boot.fip
endef

ifdef CONFIG_TARGET_ramips_mt7621
define Build/Prepare
	$(call Build/Prepare/Default)
ifdef CONFIG_DEBUG
	$(CP) $(DL_DIR)/mt7621_stage_sram.bin $(PKG_BUILD_DIR)/
else
	$(CP) $(DL_DIR)/mt7621_stage_sram_noprint.bin $(PKG_BUILD_DIR)/mt7621_stage_sram.bin
endif
endef
endif

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/Compile
	$(call Build/Compile/U-Boot)
ifeq ($(UBOOT_IMAGE),u-boot.fip))
	$(call Build/fip-image)
endif
endef

# don't stage files to bindir, let target/linux/mediatek/image/*.mk do that
ifdef CONFIG_TARGET_mediatek
define Package/u-boot/install
endef
endif

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
