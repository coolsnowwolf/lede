# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rkbin
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2024-02-22
PKG_SOURCE_VERSION:=a2a0b89b6c8c612dca5ed9ed8a68db8a07f68bc0
PKG_MIRROR_HASH:=49ffeae00fd5b6f53c94ae43d5377325e9284bd6ec5be3ab388512cccae7babd

PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define Build/rkbin/Target
  $(eval $(call rkbin/Default,$(1)))
  $(eval $(call rkbin/$(1),$(1)))

  define Package/rkbin-$(1)
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=Rockchip rkbin for $(1) SoCs
    DEPENDS:=$(DEPENDS)
    VARIANT:=$(1)
  endef

  define Package/rkbin-$(1)/install
  $$(Package/rkbin/install)
  endef
endef

define rkbin/rk3328
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk33/rk322xh_bl31_v1.49.elf
  TPL:=rk33/rk3328_ddr_333MHz_v1.19.bin
  SPL:=rk33/rk322xh_miniloader_v2.50.bin
endef

define rkbin/rk3399
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk33/rk3399_bl31_v1.36.elf
  TPL:=rk33/rk3399_ddr_800MHz_v1.30.bin
  SPL:=rk33/rk3399_miniloader_v1.30.bin
endef

define rkbin/rk3528
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk35/rk3528_bl31_v1.17.elf
  TPL:=rk35/rk3528_ddr_1056MHz_v1.09.bin
endef

define rkbin/rk3566
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk35/rk3568_bl31_v1.44.elf
  TPL:=rk35/rk3566_ddr_1056MHz_v1.21.bin
endef

define rkbin/rk3568
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk35/rk3568_bl31_v1.44.elf
  TPL:=rk35/rk3568_ddr_1560MHz_v1.21.bin
endef

define rkbin/rk3588
  DEPENDS:=@TARGET_rockchip_armv8
  ATF:=rk35/rk3588_bl31_v1.45.elf
  TPL:=rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin
endef

VARIANTS:= \
	rk3328 \
	rk3399 \
	rk3528 \
	rk3566 \
	rk3568 \
	rk3588

define Build/Compile
	@echo Building idbLoader from Rockchip rkbin project...
ifneq ($(SPL),)
	( \
		pushd $(PKG_BUILD_DIR) ; \
		$(SED) 's,$$$$(PKG_BUILD_DIR),$(PKG_BUILD_DIR),g' trust.ini ; \
		$(SED) 's,$$$$(VARIANT),$(BUILD_VARIANT),g' trust.ini ; \
		./tools/mkimage -n $(BUILD_VARIANT) -T rksd -d bin/$(TPL) \
			$(BUILD_VARIANT)-idbloader.bin ; \
		cat bin/$(SPL) >> $(BUILD_VARIANT)-idbloader.bin ; \
		./tools/trust_merger --replace bl31.elf bin/$(ATF) trust.ini ; \
		popd ; \
	)
endif
endef

define BuildPackage/rkbin
  $(foreach type,$(if $(DUMP),$(VARIANTS),$(BUILD_VARIANT)), \
    $(eval $(call Build/rkbin/Target,$(type)))
  )
  $(eval $(call Build/DefaultTargets))
  $(foreach type,$(if $(DUMP),$(VARIANTS),$(BUILD_VARIANT)), \
    $(call BuildPackage,rkbin-$(type))
  )
endef

define Package/rkbin/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)

	$(CP) $(PKG_BUILD_DIR)/bin/$(ATF) $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/bin/$(TPL) $(STAGING_DIR_IMAGE)/
ifneq ($(SPL),)
	$(CP) $(PKG_BUILD_DIR)/tools/loaderimage $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/$(BUILD_VARIANT)-idbloader.bin $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/$(BUILD_VARIANT)-trust.bin $(STAGING_DIR_IMAGE)/
endif
endef

$(eval $(call BuildPackage/rkbin))
