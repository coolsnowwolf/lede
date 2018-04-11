#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=ls-rcw-bin
PKG_SOURCE_DATE:=2017-11-19
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-rcw-bin.git
PKG_SOURCE_VERSION:=bf3ae3cb15829876007c553509501cdaa16745e7
PKG_MIRROR_HASH:=dc14a47e9529578c63d9ad2bf4e27ef565a055e4121ce2d1324c019f69074314

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define rcw-bin/Default
	TITLE:=
	CONFIG:=
endef

define rcw-bin/ls1012ardb
	TITLE=NXP LS1012ARDB RCW Binary
	CONFIG=ls1012ardb/R_SPNH_3508/PBL_0x35_0x08_1000_250_1000_default.bin
endef

define rcw-bin/ls1088ardb
	TITLE=NXP LS1088ARDB RCW Binary
	CONFIG=ls1088ardb/FCQQQQQQQQ_PPP_H_0x1d_0x0d/PBL_QSPI_1600_700_2100_0x1d_0x0d.bin
endef

define rcw-bin/ls2088ardb
	TITLE=NXP LS2088ARDB RCW Binary
	CONFIG=ls2088ardb/FFFFFFFF_PP_HH_0x2a_0x41/PBL_0x2a_0x41_1800_700_1866_1600.bin
endef

define rcw-bin/ls1012afrdm
	TITLE=NXP LS1012AFRDM RCW Binary
	CONFIG=ls1012afrdm/N_SSNP_3305/PBL_0x33_0x05_800_250_1000_default.bin
endef

RCWS := \
	ls1012ardb \
	ls1088ardb \
	ls2088ardb \
	ls1012afrdm

define Package/rcw-bin/template
define Package/rcw-layerscape-$(1)
	SECTION:=firmware
	CATEGORY:=Firmware
	DEPENDS:=@TARGET_layerscape
	TITLE:=$(2)
	VARIANT:=$(1)
endef
endef

define BuildRCWPackage
	$(eval $(rcw-bin/Default))
	$(eval $(rcw-bin/$(1)))
	$(call Package/rcw-bin/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call rcw-bin/$(BUILD_VARIANT)))
RCW_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
endif

define Build/Compile
endef

define Package/rcw-bin/install/default
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(RCW_CONFIG) $(STAGING_DIR_IMAGE)/$(1)-rcw.bin
endef

define Package/rcw-bin/install/template
define Package/rcw-layerscape-$(1)/install
	$(call Package/rcw-bin/install/default,$(2))
endef
endef

$(foreach r,$(RCWS), \
	$(eval $(call Package/rcw-bin/install/template,$(r),$(r))) \
)

$(foreach r,$(RCWS), \
	$(eval $(call BuildRCWPackage,$(r))) \
	$(eval $(call BuildPackage,rcw-layerscape-$(r))) \
)
