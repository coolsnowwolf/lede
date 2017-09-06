#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ppfe
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=645d9aaaaa39ac1fa50cf805f1866feb5f717b34cb110c80e3f45303ea6d7259
PKG_SOURCE_URL:=https://github.com/fsl-jyt/ppfe.git
PKG_SOURCE_VERSION:=b9bb5cc690c0aab2592eb9d443ef55cc9dff8557
PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define ppfe/Default
	TITLE:=
	CONFIG:=
endef

define ppfe/ls1012ardb
	TITLE:=PPFE firmware $(PKG_NAME)-$(PKG_VERSION) for NXP ls1012a Dev Board\
			(must enable CONFIG_NO_STRIP=y for avoid firmware be broke)
	CONFIG:=ls1012a/Freescale-Binary-EULA ls1012a/ppfe_class_ls1012a.elf ls1012a/ppfe_tmu_ls1012a.elf
endef

PPFES := \
	ls1012ardb

define Package/ppfe/template
define Package/ppfe-$(1)
	SECTION:=firmware
	CATEGORY:=Firmware
	DEPENDS:=@TARGET_layerscape
	TITLE:=$(2)
	URL:=https://github.com/fsl-jyt/ppfe.git
	VARIANT:=$(1)
	MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>
endef
endef

define BuildPPFEPackage
	$(eval $(ppfe/Default))
	$(eval $(ppfe/$(1)))
	$(call Package/ppfe/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call ppfe/$(BUILD_VARIANT)))
PPFE_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
endif

define Build/Compile
endef

define Package/ppfe/install/default
	$(INSTALL_DIR) $(1)/lib/firmware
	$(foreach i,$(PPFE_CONFIG), \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(i) $(1)/lib/firmware/ ;\
	)
endef

define Package/ppfe/install/template
define Package/ppfe-$(1)/install
	$(call Package/ppfe/install/default,$$(1),$(2))
endef
endef

$(foreach p,$(PPFES), \
	$(eval $(call Package/ppfe/install/template,$(p),$(p))) \
)

$(foreach p,$(PPFES), \
	$(eval $(call BuildPPFEPackage,$(p))) \
	$(eval $(call BuildPackage,ppfe-$(p))) \
)
