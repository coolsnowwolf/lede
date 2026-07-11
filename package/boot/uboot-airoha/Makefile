include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2026.07
PKG_HASH:=78e8bfc382fe388f9b55aa1daf8c563522a037779b5d4c349d1415e381f1243e
PKG_BUILD_DEPENDS:=arm-trusted-firmware-tools/host

UBOOT_USE_INTREE_DTC:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=airoha
  LZMA_COMPRESS:=1
endef

define U-Boot/an7581_gemtek_w1700k
  NAME:=Gemtek W1700K (UBI)
  UBOOT_CONFIG:=an7581_w1700k
  BUILD_DEVICES:=gemtek_w1700k-ubi
  BUILD_SUBTARGET:=an7581
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/an7581_nokia_valyrian
  NAME:=Nokia Valyrian
  UBOOT_CONFIG:=an7581_nokia_valyrian
  BUILD_DEVICES:=nokia_valyrian
  BUILD_SUBTARGET:=an7581
  UBOOT_IMAGE:=u-boot.fip
  BL2_IMAGE:=an7581-bl2.bin
  BL31_IMAGE:=an7581-bl31.bin
endef

define U-Boot/an7581_nokia_xg-040g-md
  NAME:=Nokia XG-040G-MD
  UBOOT_CONFIG:=an7581_nokia_xg-040g-md
  BUILD_DEVICES:=nokia_xg-040g-md-ubi
  BUILD_SUBTARGET:=an7581
  UBOOT_IMAGE:=u-boot.fip
  BL2_IMAGE:=an7581-bl2.bin
  BL31_IMAGE:=an7581-bl31.bin
endef

define U-Boot/an7581_rfb
  NAME:=AN7581 Reference Board
  UBOOT_CONFIG:=an7581_evb
  BUILD_DEVICES:=airoha_an7581-evb
  BUILD_SUBTARGET:=an7581
  UBOOT_IMAGE:=u-boot.fip
  BL2_IMAGE:=an7581-bl2.bin
  BL31_IMAGE:=an7581-bl31.bin
endef

define U-Boot/an7583_rfb
  NAME:=AN7583 Reference Board
  UBOOT_CONFIG:=an7583_evb
  BUILD_DEVICES:=airoha_an7583-evb
  BUILD_SUBTARGET:=an7583
  UBOOT_IMAGE:=u-boot.fip
  BL2_IMAGE:=an7583-bl2.bin
  BL31_IMAGE:=an7583-bl31.bin
endef

define U-Boot/en7523_rfb
  NAME:=EN7523 Reference Board
  UBOOT_CONFIG:=en7523_evb
  BUILD_DEVICES:=airoha_en7523-evb
  BUILD_SUBTARGET:=en7523
  UBOOT_IMAGE:=u-boot.fip
  BL2_IMAGE:=en7523-bl2.bin
  BL31_IMAGE:=en7523-bl31.bin
endef

UBOOT_TARGETS := \
	an7581_gemtek_w1700k \
	an7581_nokia_valyrian \
	an7581_nokia_xg-040g-md \
	an7581_rfb \
	an7583_rfb \
	en7523_rfb

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_KWBIMAGE \
	--disable TOOLS_LIBCRYPTO \
	--disable TOOLS_MKEFICAPSULE \
	--enable SERIAL_RX_BUFFER \
	--set-val SERIAL_RX_BUFFER_SIZE 256

define Build/fip-image-bl2
	$(STAGING_DIR_HOST)/bin/fiptool create \
		--tb-fw files/$(BL2_IMAGE) \
		$(PKG_BUILD_DIR)/bl2.fip
endef

define Build/fip-image
	$(if $(LZMA_COMPRESS), $(STAGING_DIR_HOST)/bin/lzma e \
		files/$(BL31_IMAGE) \
		$(PKG_BUILD_DIR)/bl31.bin.lzma)
	$(STAGING_DIR_HOST)/bin/fiptool create \
		--soc-fw $(PKG_BUILD_DIR)/bl31.bin$(if $(LZMA_COMPRESS),.lzma) \
		--nt-fw $(PKG_BUILD_DIR)/u-boot.bin$(if $(LZMA_COMPRESS),.lzma) \
		$(PKG_BUILD_DIR)/u-boot.fip
endef

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/Compile
	$(call Build/Compile/U-Boot)
	$(if $(LZMA_COMPRESS), $(STAGING_DIR_HOST)/bin/lzma e \
		$(PKG_BUILD_DIR)/u-boot.bin \
		$(PKG_BUILD_DIR)/u-boot.bin.lzma)
ifeq ($(UBOOT_IMAGE),u-boot.fip)
	$(call Build/fip-image-bl2)
	$(call Build/fip-image)
endif
endef

# don't stage files to bindir, let target/linux/airoha/image/*.mk do that
define Package/u-boot/install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifeq ($(UBOOT_IMAGE),u-boot.fip)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bl2.fip $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.fip
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.fip $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl31-u-boot.fip
else
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.bin$(if $(LZMA_COMPRESS),.lzma) \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.bin$(if $(LZMA_COMPRESS),.lzma)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.dtb \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.dtb
endif
endef

$(eval $(call BuildPackage/U-Boot))
