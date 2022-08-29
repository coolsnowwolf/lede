include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2021.04
PKG_HASH:=0d438b1bb5cceb57a18ea2de4a0d51f7be5b05b98717df05938636e0aadfe11a
PKG_BUILD_DEPENDS:=arm-trusted-firmware-tools/host

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=mediatek
  UBOOT_IMAGE:=u-boot-mtk.bin
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
  BUILD_DEVICES:=ubnt_unifi-6-lr-ubootmod
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-nor-2ddr
  FIP_COMPRESS:=1
endef

define U-Boot/mt7623a_unielec_u7623
  NAME:=UniElec U7623 (mt7623)
  BUILD_DEVICES:=unielec_u7623-emmc unielec_u7623-02-emmc-512m-legacy
  BUILD_SUBTARGET:=mt7623
  UBOOT_CONFIG:=mt7623a_unielec_u7623_02
endef

define U-Boot/mt7623n_bpir2
  NAME:=Banana Pi R2 (mt7623)
  BUILD_DEVICES:=bpi_bananapi-r2
  BUILD_SUBTARGET:=mt7623
  UBOOT_IMAGE:=u-boot.bin
  UBOOT_CONFIG:=mt7623n_bpir2
endef

define U-Boot/mt7629_rfb
  NAME:=MT7629 Reference Board
  BUILD_SUBTARGET:=mt7629
  BUILD_DEVICES:=mediatek_mt7629-rfb
  UBOOT_CONFIG:=mt7629_rfb
endef

UBOOT_TARGETS := \
	mt7622_bananapi_bpi-r64-emmc \
	mt7622_bananapi_bpi-r64-sdmmc \
	mt7622_bananapi_bpi-r64-snand \
	mt7622_linksys_e8450 \
	mt7622_rfb1 \
	mt7622_ubnt_unifi-6-lr \
	mt7623n_bpir2 \
	mt7623a_unielec_u7623 \
	mt7629_rfb

UBOOT_MAKE_FLAGS += $(UBOOT_IMAGE:.fip=.bin)

Build/Exports:=$(Host/Exports)

define Build/fip-image
	$(if $(FIP_COMPRESS),\
		xz -f -e -k -9 -C crc32 $(STAGING_DIR_IMAGE)/$(BUILD_SUBTARGET)-$(BL2_BOOTDEV)-$(BL2_DDRBLOB)ddr-bl31.bin ;\
		xz -f -e -k -9 -C crc32 $(PKG_BUILD_DIR)/u-boot.bin \
	)
	$(STAGING_DIR_HOST)/bin/fiptool create \
		--soc-fw $(STAGING_DIR_IMAGE)/$(BUILD_SUBTARGET)-$(BL2_BOOTDEV)-$(BL2_DDRBLOB)ddr-bl31.bin$(if $(FIP_COMPRESS),.xz) \
		--nt-fw $(PKG_BUILD_DIR)/u-boot.bin$(if $(FIP_COMPRESS),.xz) \
		$(PKG_BUILD_DIR)/u-boot.fip
endef

define Build/Compile
	$(call Build/Compile/U-Boot)
ifeq ($(UBOOT_IMAGE),u-boot.fip))
	$(call Build/fip-image)
endif
endef

# don't stage files to bindir, let target/linux/mediatek/image/*.mk do that
define Package/u-boot/install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
