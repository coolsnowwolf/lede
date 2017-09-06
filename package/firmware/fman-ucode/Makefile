#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=fman-ucode
PKG_VERSION:=fsl-sdk-v2.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=209c60e4efaa3ce9c9a954266a7210fa0047a2de47ce00a412aa0318d94d0d88
PKG_SOURCE_URL:=https://github.com/fsl-jyt/fm-ucode.git
PKG_SOURCE_VERSION:=b19c645821941493fbef32e616b5a16404259976

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define fman/Default
	TITLE:=
	CONFIG:=
endef

define fman/ls1043ardb
	TITLE=FMAN UCODE $(PKG_NAME)-$(PKG_VERSION) for NXP ls1043ardb 64b/32b Dev Board
	CONFIG=fsl_fman_ucode_ls1043_r1.0_108_4_5.bin
endef

define fman/ls1046ardb
	TITLE=FMAN UCODE $(PKG_NAME)-$(PKG_VERSION) for NXP ls1046ardb 64b/32b Dev Board
	CONFIG=fsl_fman_ucode_t2080_r1.1_106_4_18.bin
endef

FMANS := \
	ls1043ardb \
	ls1046ardb

define Package/fman/template
define Package/fman-layerscape-$(1)
	SECTION:=firmware
	CATEGORY:=Firmware
	DEPENDS:=@TARGET_layerscape
	TITLE:=$(2)
	URL:=http://git.freescale.com/git/cgit.cgi/ppc/sdk/fm-ucode.git
	VARIANT:=$(1)
	MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>
endef
endef

define BuildFMANPackage
	$(eval $(fman/Default))
	$(eval $(fman/$(1)))
	$(call Package/fman/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call fman/$(BUILD_VARIANT)))
FMAN_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
endif

define Build/Compile
endef

define Package/fman/install/default
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(FMAN_CONFIG) $(STAGING_DIR_IMAGE)/$(1)-fman.bin
endef

define Package/fman/install/template
define Package/fman-layerscape-$(1)/install
	$(call Package/fman/install/default,$(2))
endef
endef

$(foreach f,$(FMANS), \
	$(eval $(call Package/fman/install/template,$(f),$(f))) \
)

$(foreach f,$(FMANS), \
	$(eval $(call BuildFMANPackage,$(f))) \
	$(eval $(call BuildPackage,fman-layerscape-$(f))) \
)
