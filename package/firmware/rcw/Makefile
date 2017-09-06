#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=rcw
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=8082979dbdef57e36a3096986aa426ef678c152403eb1dbd132da28f01490b6d
PKG_SOURCE_URL:=https://github.com/fsl-jyt/rcw.git
PKG_SOURCE_VERSION:=17e09e19bd5cf94e5ebc84916b4607743c29723e

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define rcw/Default
	TITLE:=
	CONFIG:=
endef

define rcw/ls1043ardb
	TITLE=RCW binary $(PKG_NAME)-$(PKG_VERSION) for NXP ls1043ardb 64b/32b Dev Board
	CONFIG=ls1043ardb-rcw.bin
endef

define rcw/ls1046ardb
	TITLE=RCW binary $(PKG_NAME)-$(PKG_VERSION) for NXP ls1046ardb 64b/32b Dev Board
	CONFIG=ls1046ardb-rcw.bin
endef

define rcw/ls1012ardb
	TITLE=RCW binary $(PKG_NAME)-$(PKG_VERSION) for NXP ls1012ardb 64b/32b Dev Board
	CONFIG=ls1012ardb-rcw.bin
endef

RCWS := \
	ls1043ardb \
	ls1046ardb \
	ls1012ardb

define Package/rcw/template
define Package/rcw-layerscape-$(1)
	SECTION:=firmware
	CATEGORY:=Firmware
	DEPENDS:= @TARGET_layerscape
	TITLE:=$(2)
	URL:=http://git.freescale.com/git/cgit.cgi/ppc/sdk/rcw.git
	VARIANT:=$(1)
	MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>
endef
endef

define BuildRCWPackage
	$(eval $(rcw/Default))
	$(eval $(rcw/$(1)))
	$(call Package/rcw/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call rcw/$(BUILD_VARIANT)))
RCW_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
endif

define Build/Configure
endef

define Build/Compile
endef

define Package/rcw/install/default
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(RCW_CONFIG) $(STAGING_DIR_IMAGE)/$(1)-rcw.bin
endef

define Package/rcw/install/template
define Package/rcw-layerscape-$(1)/install
	$(call Package/rcw/install/default,$(2))
endef
endef

$(foreach r,$(RCWS), \
	$(eval $(call Package/rcw/install/template,$(r),$(r))) \
)

$(foreach r,$(RCWS), \
	$(eval $(call BuildRCWPackage,$(r))) \
	$(eval $(call BuildPackage,rcw-layerscape-$(r))) \
)
