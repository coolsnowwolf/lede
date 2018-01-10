#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=uboot-layerscape
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=6aba7cb32d655dacb49bd188706bf1284159ae9b1fa69056df634b90070c3768
PKG_SOURCE_URL:=https://github.com/fsl-jyt/u-boot.git
PKG_SOURCE_VERSION:=aefcbf4c2bb36e1f0ead61181e5fe1119b5683ad

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
	TITLE:=
	CONFIG:=
	IMAGE:=
endef

define uboot/ls1043ardb
	TITLE:=U-Boot $(PKG_NAME)-$(PKG_VERSION) for NXP ls1043ardb 64b Dev Board
	CONFIG=ls1043ardb
endef

define uboot/ls1046ardb
	TITLE:=U-Boot $(PKG_NAME)-$(PKG_VERSION) for NXP ls1046ardb 64b Dev Board
	CONFIG=ls1046ardb_qspi
endef

define uboot/ls1012ardb
	TITLE:=U-Boot $(PKG_NAME)-$(PKG_VERSION) for NXP ls1012ardb 64b Dev Board
	CONFIG=ls1012ardb_qspi
	IMAGE=u-boot-swap.bin
endef

UBOOTS := \
	ls1043ardb \
	ls1046ardb \
	ls1012ardb

define Package/uboot/template
define Package/uboot-layerscape-64b-$(1)
	SECTION:=boot
	CATEGORY:=Boot Loaders
	DEPENDS:= @TARGET_layerscape_64b
	TITLE:=$(2)
	URL:=http://git.freescale.com/git/cgit.cgi/ppc/sdk/u-boot.git/snapshot
	VARIANT:=$(1)
	MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),u-boot-dtb.bin)
endif

define Build/Configure
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(UBOOT_CONFIG)_config
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) DTC=$(LINUX_DIR)/scripts/dtc/dtc
endef

define Package/uboot/install/default
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(KDIR)/$(1)-64b-uboot.bin
endef

define Package/uboot/install/template
define Package/uboot-layerscape-64b-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-layerscape-64b-$(u))) \
)
