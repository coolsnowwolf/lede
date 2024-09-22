#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2023.07
PKG_RELEASE:=$(AUTORELEASE)

PKG_HASH:=12e921b466ae731cdbc355e6832b7f22bc90b01aeceef9886f98aaba7b394300

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
endef


# RK3328 boards

define U-Boot/rk3328/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3328
  ATF:=rk322xh_bl31_v1.49.elf
  USE_RKBIN:=1
endef

define U-Boot/nanopi-r2c-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=NanoPi R2C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c
endef

define U-Boot/nanopi-r2s-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=NanoPi R2S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2s \
    friendlyarm_nanopi-neo3
endef

define U-Boot/orangepi-r1-plus-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=Orange Pi R1 Plus
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus
endef

define U-Boot/orangepi-r1-plus-lts-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=Orange Pi R1 Plus LTS
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus-lts
endef


# RK3399 boards

define U-Boot/rk3399/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3399
  ATF:=rk3399_bl31.elf
endef

define U-Boot/dlfr100-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Dilusense DLFR100
  BUILD_DEVICES:= \
    dilusense_dlfr100
endef

define U-Boot/fine3399-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Fine 3399
  BUILD_DEVICES:= \
    rumu3f_fine-3399
endef

define U-Boot/h3399pc-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=SHAREVDI H3399PC
  BUILD_DEVICES:= \
    sharevdi_h3399pc
endef

define U-Boot/king3399-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Rongpin King3399
  BUILD_DEVICES:= \
    rongpin_king3399
endef

define U-Boot/mpc1903-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Rocktech MPC1903
  BUILD_DEVICES:= \
    rocktech_mpc1903
endef

define U-Boot/nanopc-t4-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPC T4
  BUILD_DEVICES:= \
    friendlyarm_nanopc-t4
endef

define U-Boot/nanopi-r4s-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3399
  ATF:=rk3399_bl31_v1.36.elf
  USE_RKBIN:=1
endef

define U-Boot/nanopi-r4se-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4SE
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4se
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3399
  ATF:=rk3399_bl31_v1.36.elf
  USE_RKBIN:=1
endef

define U-Boot/rock-pi-4-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Rock Pi 4
  BUILD_DEVICES:= \
    radxa_rock-pi-4
endef

define U-Boot/rockpro64-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
endef

define U-Boot/xiaobao-nas-v1-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Codinge Xiaobao NAS-I
  BUILD_DEVICES:= \
    codinge_xiaobao-nas-v1
endef

define U-Boot/guangmiao-g4c-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=GuangMiao G4C
  BUILD_DEVICES:= \
    sharevdi_guangmiao-g4c
endef

define U-Boot/sv901-eaio-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=SV901 EAIO
  BUILD_DEVICES:= \
    scensmart_sv901-eaio
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3399
  ATF:=rk3399_bl31_v1.36.elf
  USE_RKBIN:=1
endef

# RK3566 boards

define U-Boot/rk3566/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3566
  ATF:=rk3568_bl31_v1.44.elf
  TPL:=rk3566_ddr_1056MHz_v1.21.bin
endef

define U-Boot/panther-x2-rk3566
  $(U-Boot/rk3566/Default)
  NAME:=Panther X2
  BUILD_DEVICES:= \
    panther_x2
endef

define U-Boot/rock-3c-rk3566
  $(U-Boot/rk3566/Default)
  NAME:=ROCK 3C
  BUILD_DEVICES:= \
    radxa_rock-3c
endef

# RK3568 boards

define U-Boot/rk3568/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3568
  ATF:=rk3568_bl31_v1.44.elf
  TPL:=rk3568_ddr_1560MHz_v1.21.bin
endef

define U-Boot/lyt-t68m-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=LYT T68M
  BUILD_DEVICES:= lyt_t68m
endef

define U-Boot/mrkaio-m68s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Mrkaio M68S
  BUILD_DEVICES:= \
    ezpro_mrkaio-m68s \
    ezpro_mrkaio-m68s-plus
endef

define U-Boot/nanopi-r5s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=NanoPi R5S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r5c \
    friendlyarm_nanopi-r5s
endef

define U-Boot/opc-h68k-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=OPC-H68K Board
  BUILD_DEVICES:= \
    hinlink_opc-h66k \
    hinlink_opc-h68k \
    hinlink_opc-h69k
endef

define U-Boot/photonicat-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Ariaboard Photonicat
  BUILD_DEVICES:= \
    ariaboard_photonicat
endef

define U-Boot/r66s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=R66S/R68S
  BUILD_DEVICES:= \
    fastrhino_r66s \
    fastrhino_r68s
endef

define U-Boot/radxa-e25-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Radxa E25
  BUILD_DEVICES:= \
    radxa_e25
endef

define U-Boot/rock-3a-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=ROCK 3A
  BUILD_DEVICES:= \
    radxa_rock-3a
endef

define U-Boot/rsb4810-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Advantech RSB4810
  BUILD_DEVICES:= \
    advantech_rsb4810
endef

define U-Boot/seewo-sv21-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=seewo SV21
  BUILD_DEVICES:= \
    seewo_sv21
endef

define U-Boot/sige3-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Sige3
  BUILD_DEVICES:= \
    armsom_sige3
endef

define U-Boot/station-p2-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=StationP2
  BUILD_DEVICES:= \
    firefly_station-p2
endef

# RK3588 boards

define U-Boot/Default/rk3588
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):rkbin-rk3588
  ATF:=rk3588_bl31_v1.45.elf
  TPL:=rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin
endef

define U-Boot/sige7-rk3588
  $(U-Boot/Default/rk3588)
  NAME:=Sige7
  BUILD_DEVICES:= \
    armsom_sige7
endef

UBOOT_TARGETS := \
  panther-x2-rk3566 \
  rock-3c-rk3566 \
  lyt-t68m-rk3568 \
  mrkaio-m68s-rk3568 \
  nanopi-r5s-rk3568 \
  opc-h68k-rk3568 \
  photonicat-rk3568 \
  r66s-rk3568 \
  radxa-e25-rk3568 \
  rock-3a-rk3568 \
  rsb4810-rk3568 \
  seewo-sv21-rk3568 \
  sige3-rk3568 \
  station-p2-rk3568 \
  dlfr100-rk3399 \
  fine3399-rk3399 \
  h3399pc-rk3399 \
  king3399-rk3399 \
  mpc1903-rk3399 \
  nanopc-t4-rk3399 \
  nanopi-r4s-rk3399 \
  nanopi-r4se-rk3399 \
  rock-pi-4-rk3399 \
  rockpro64-rk3399 \
  sv901-eaio-rk3399 \
  guangmiao-g4c-rk3399 \
  xiaobao-nas-v1-rk3399 \
  nanopi-r2c-rk3328 \
  nanopi-r2s-rk3328 \
  orangepi-r1-plus-rk3328 \
  orangepi-r1-plus-lts-rk3328 \
  sige7-rk3588

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF) \
  PATH=$(STAGING_DIR_HOST)/bin:$(PATH) \
  $(if $(TPL),ROCKCHIP_TPL=$(STAGING_DIR_IMAGE)/$(TPL))

define Build/Configure
	$(call Build/Configure/U-Boot)

	$(SED) 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
	$(SED) 's#CONFIG_MKIMAGE_DTC_PATH=.*#CONFIG_MKIMAGE_DTC_PATH="$(PKG_BUILD_DIR)/scripts/dtc/dtc"#g' $(PKG_BUILD_DIR)/.config
	echo 'CONFIG_IDENT_STRING=" OpenWrt"' >> $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifneq ($(USE_RKBIN),)
	$(STAGING_DIR_IMAGE)/loaderimage --pack --uboot $(PKG_BUILD_DIR)/u-boot-dtb.bin $(PKG_BUILD_DIR)/uboot.img 0x200000
	$(CP) $(PKG_BUILD_DIR)/uboot.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.img
else
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
endif
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
