# Copyright (C) 2010 OpenWrt.org
# Copyright (C) 2015-2016 Lantiq Beteiligungs GmbH & Co KG.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-vdsl-app
PKG_VERSION:=4.17.18.6
PKG_RELEASE:=8
PKG_BASE_NAME:=dsl_cpe_control
PKG_SOURCE:=$(PKG_BASE_NAME)_vrx-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@OPENWRT
PKG_HASH:=da8bb929526a61aea0e153ef524331fcd472a1ebbc6d88ca017735a4f82ece02
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_BASE_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS:=ltq-vdsl

PKG_FLAGS:=nonshared
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/ltq-vdsl-app
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Lantiq VDSL userland tool
  URL:=http://www.lantiq.com/
  DEPENDS:=@TARGET_lantiq_xrx200 +libpthread +librt +ltq-dsl-base +libubox +libubus
endef

define Package/ltq-vdsl-app/description
  Userland tool needed to control Lantiq VDSL CPE
endef

# ltq-vdsl-app uses a header provided by the MEI driver which has some
# conditionals.
#
# Define them here with the default values they would get in the MEI driver,
# have the same view on both sides.
#
# If you change them, you need to change them for the ltq-vdsl-app as well
VDSL_APP_CFLAGS = \
	-DMAX_CLI_PIPES=1 \
	-DMEI_SUPPORT_DEBUG_STREAMS=1 \
	-DMEI_SUPPORT_OPTIMIZED_FW_DL=1

CONFIGURE_ARGS += \
	--enable-vrx \
	--enable-vrx-device=vr9 \
	--enable-driver-include="-I$(STAGING_DIR)/usr/include/drv_vdsl_cpe_api" \
	--enable-device-driver-include="-I$(STAGING_DIR)/usr/include/vdsl/" \
	--enable-ifxos \
	--enable-ifxos-include="-I$(STAGING_DIR)/usr/include/ifxos" \
	--enable-ifxos-library="-I$(STAGING_DIR)/usr/lib" \
	--enable-add-appl-cflags="$(VDSL_APP_CFLAGS)"  \
	--enable-debug \
	--disable-dti \
	--with-channels-per-line="1"

#CONFIGURE_ARGS += --enable-model=full
#CONFIGURE_ARGS += --enable-model=lite
#CONFIGURE_ARGS += --enable-model=footprint
CONFIGURE_ARGS += \
	--enable-model=typical \
	--enable-dsl-pm-showtime \
	--disable-dsl-ceoc \
	--enable-dsl-pm-retx-counters \
	--enable-dsl-pm-retx-thresholds
#CONFIGURE_ARGS += --enable-model=debug

define Package/ltq-vdsl-app/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/sbin $(1)/etc/hotplug.d/dsl
	$(INSTALL_BIN) ./files/dsl_control $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/10_atm.sh $(1)/etc/hotplug.d/dsl
	$(INSTALL_BIN) ./files/10_ptm.sh $(1)/etc/hotplug.d/dsl

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dsl_cpe_control $(1)/sbin/vdsl_cpe_control
	$(INSTALL_BIN) ./files/dsl_cpe_pipe.sh $(1)/sbin/
endef

$(eval $(call BuildPackage,ltq-vdsl-app))
