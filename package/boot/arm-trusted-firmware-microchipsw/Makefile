include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-microchipsw
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/microchip-ung/arm-trusted-firmware.git
PKG_SOURCE_DATE:=2026-01-07
PKG_SOURCE_VERSION:=7696c9aaaae7c677f4c373a61a1289cba7f824aa
PKG_MIRROR_HASH:=501fbc8b97a530300de2fc25e893c45e5620ad97d19069c25f593290d3f96bf0

PKG_BUILD_DEPENDS:=ruby/host
PKG_MAINTAINER:=Robert Marko <robert.marko@sartura.hr>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=microchipsw
  TFA_IMAGE:=fip.bin fwu_fip.bin fwu.html
endef

define Trusted-Firmware-A/ev23x71a
  NAME:=Microchip EV23X71A
  BUILD_SUBTARGET:=lan969x
  BUILD_DEVICES:=microchip_ev23x71a
  PLAT:=lan969x_a0
  DEPENDS:=+u-boot-ev23x71a
endef

define Trusted-Firmware-A/tactical-1000
  NAME:=Novarq Tactical 1000
  BUILD_SUBTARGET:=lan969x
  BUILD_DEVICES:=novarq_tactical-1000
  PLAT:=novarq_tactical_1000_v3
  DEPENDS:=+u-boot-tactical-1000
endef

TFA_TARGETS:= \
	ev23x71a \
	tactical-1000

MBEDTLS_NAME:=mbedtls
MBEDTLS_RELEASE:=2.28.10
MBEDTLS_SOURCE:=$(MBEDTLS_NAME)-$(MBEDTLS_RELEASE).tar.zst

define Download/mbedtls
  FILE:=$(MBEDTLS_SOURCE)
  PROTO:=git
  URL:=https://github.com/Mbed-TLS/mbedtls.git
  SOURCE_VERSION:=2fc8413bfcb51354c8e679141b17b3f1a5942561
  MIRROR_HASH:=40b94a76572ad1ca89738929ab81d6024f678f22691eb3bd633c076ac18a334a
  SUBDIR:=$(MBEDTLS_NAME)
endef

define Build/Prepare
	# Download mbedtls
	$(eval $(call Download,mbedtls))

	$(call Build/Prepare/Default,)

	$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(MBEDTLS_SOURCE)
endef

TFA_MAKE_FLAGS += \
	MBEDTLS_DIR=$(PKG_BUILD_DIR)/$(MBEDTLS_NAME) \
	BL33=$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.bin \
	KEY_ALG=ecdsa GENERATE_COT=1 TRUSTED_BOARD_BOOT=1 \
	all fip fwu_fip

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/fip.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-fip.bin
	$(CP) $(patsubst %,$(PKG_BUILD_DIR)/build/$(PLAT)/release/%,$(TFA_IMAGE)) $(1)/
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
