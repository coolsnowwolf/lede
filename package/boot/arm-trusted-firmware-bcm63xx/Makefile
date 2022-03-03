# SPDX-License-Identifier: GPL-2.0

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/bcm63xx/atf.git
PKG_SOURCE_DATE:=2021-12-24
PKG_SOURCE_VERSION:=e6d46baf3fae79f693f90bf34f7284c3dfc64aef
PKG_MIRROR_HASH:=9d5d04f572b1b6ddc6eb3064b9cb09f5fe982e82d350790041d35316349af124

PKG_MAINTAINER:=Rafał Miłecki <rafal@milecki.pl>

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  PLAT:=bcm
  DEFAULT:=y
endef

define Trusted-Firmware-A/bcm4908
  BUILD_TARGET:=bcm4908
  NAME:=BCM4908
  BRCM_CHIP=4908
  TFA_IMAGE:=bl31.bin
endef

TFA_TARGETS:= \
	bcm4908

TFA_MAKE_FLAGS += \
	BRCM_CHIP=$(BRCM_CHIP)

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/$(TFA_IMAGE) $(STAGING_DIR_IMAGE)/
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
