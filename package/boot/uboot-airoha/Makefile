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
  HIDDEN:=1
endef

define U-Boot/an7581_rfb
  NAME:=AN7581 Reference Board
  UBOOT_CONFIG:=an7581_evb
  BUILD_DEVICES:=airoha_an7581-evb airoha_an7581-evb-emmc-eagle airoha_an7581-evb-emmc-kite
  BUILD_SUBTARGET:=an7581
  BL2_IMAGE:=an7581-bl2.bin
  BL31_IMAGE:=an7581-bl31.bin
  DEPENDS:=+trusted-firmware-a-an7581-bl31
endef

define U-Boot/an7583_rfb
  NAME:=AN7583 Reference Board
  UBOOT_CONFIG:=an7583_evb
  BUILD_DEVICES:=airoha_an7583-evb airoha_an7583-evb-emmc
  BUILD_SUBTARGET:=an7583
  BL2_IMAGE:=an7583-bl2.bin
  BL31_IMAGE:=an7583-bl31.bin
  DEPENDS:=+trusted-firmware-a-an7583-bl31
endef

UBOOT_TARGETS := an7581_rfb an7583_rfb

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_KWBIMAGE \
	--disable TOOLS_LIBCRYPTO \
	--disable TOOLS_MKEFICAPSULE \
	--enable DM_RNG \
	--enable RNG_AIROHA \
	--enable SERIAL_RX_BUFFER \
	--set-val SERIAL_RX_BUFFER_SIZE 256

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/Compile
	$(call Build/Compile/U-Boot)
	$(if $(LZMA_COMPRESS), $(STAGING_DIR_HOST)/bin/lzma e \
		$(PKG_BUILD_DIR)/u-boot.bin \
		$(PKG_BUILD_DIR)/u-boot.lzma)
endef

# don't stage files to bindir, let target/linux/airoha/image/*.mk do that
define Package/u-boot/install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.$(if $(LZMA_COMPRESS),lzma,bin) \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.$(if $(LZMA_COMPRESS),lzma,bin)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.dtb \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.dtb
endef

$(eval $(call BuildPackage/U-Boot))
