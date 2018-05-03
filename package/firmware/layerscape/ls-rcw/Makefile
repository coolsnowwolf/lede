#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=ls-rcw
PKG_SOURCE_DATE:=2017-12-01
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qoriq-open-source/rcw.git
PKG_SOURCE_VERSION:=cbd5e66fcb850e821c8632578410ceba5733a49e
PKG_MIRROR_HASH:=4caad5e0ae2018a0a594f36e46e4a96698420956c64bb73656f380fd46733c3d

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define rcw/Default
	TITLE:=
	CONFIG:=
endef

define rcw/ls1043ardb
	TITLE=RCW binary $(PKG_VERSION) for NXP ls1043ardb 64b/32b Dev Board
	CONFIG=ls1043ardb/RR_FQPP_1455/rcw_1600.bin
endef

define rcw/ls1046ardb
	TITLE=RCW binary $(PKG_VERSION) for NXP ls1046ardb 64b/32b Dev Board
	CONFIG=ls1046ardb/RR_FFSSPPPH_1133_5559/rcw_1800_qspiboot.bin.swap
endef

RCWS := \
	ls1043ardb \
	ls1046ardb

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
