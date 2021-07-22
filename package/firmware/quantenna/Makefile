#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quantenna
PKG_VERSION:=37.3.2.44
PKG_RELEASE:=1

PKG_MAINTAINER:=Bjørn Mork <bjorn@mork.no>

# available versions, ref https://kb.netgear.com/2649/NETGEAR-Open-Source-Code-for-Programmers-GPL
# qtn sdk v36.6.0.23: 1.0.0.46,
# qtn sdk v36.6.0.28: 1.0.0.52,
# qtn sdk v36.6.0.30: 1.0.0.68, 1.0.0.70,
# qtn sdk v36.7.3.23: 1.0.0.76, 1.0.0.82,
# qtn sdk v37.3.1.25: 1.0.0.94, 1.0.0.108, 1.0.0.110 , 1.0.0.112, 1.0.0.116, 1.0.0.122,
# qtn sdk v37.3.2.44: 1.0.0.124

NETGEAR_R7500V1_SRC-37.3.2.44=R7500-and_qtn_gpl_src_v1.0.0.124.tar.bz2.zip

NETGEAR_R7500V1_HASH-37.3.2.44=6592e3286c66a8b0ba689e6cff2d10cac5aea3ddef8ca8746d7771319e47112e

PKG_SOURCE:=$(NETGEAR_R7500V1_SRC-$(strip $(PKG_VERSION)))
PKG_SOURCE_URL:=https://www.downloads.netgear.com/files/GPL
PKG_HASH:=$(NETGEAR_R7500V1_HASH-$(strip $(PKG_VERSION)))

QCSAPI_ARCHIVE-37.3.2.44=quantenna_gpl_v$(PKG_VERSION).tar.bz2

UNPACK_CMD-37.3.2.44=unzip -q -p $(DL_DIR)/$(PKG_SOURCE) | bzcat | $(HOST_TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)
UNPACK_CMD=$(UNPACK_CMD-$(strip $(PKG_VERSION)))

PKG_BUILD_DEPENDS:=libtirpc

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	$(PKG_UNPACK)
	bzcat $(PKG_BUILD_DIR)/$(QCSAPI_ARCHIVE-$(strip $(PKG_VERSION))) | \
		$(HOST_TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS) --strip-components=1 \
		--wildcards */libqcsapi_client_src.zip --wildcards *drivers/pcie2*
	bzcat $(PKG_BUILD_DIR)/R7500-*.tar.bz2 | \
		$(HOST_TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS) --strip-components=4 \
		--wildcards */package/qt-wireless/binary/{topaz-linux.lzma.img,u-boot.bin}
	unzip -q -d $(PKG_BUILD_DIR) $(PKG_BUILD_DIR)/libqcsapi_client_src.zip
	# tunning to make the same patch apply to versions with or without -fstrict-aliasing
	sed -i -e 's/ -fstrict-aliasing//' $(PKG_BUILD_DIR)/Makefile
	$(Build/Patch)
endef

define Package/qtn-utils
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Quantenna QV840 (QTS1000) Utilities
  DEPENDS:=+libtirpc +tc +kmod-sched-act-vlan
endef

define Package/qtn-utils/description
  Management tools for Quantenna QV840 (QTS1000) WiFi modules
endef

TARGET_CFLAGS += $(FPIC) -I$(STAGING_DIR)/usr/include/tirpc
TARGET_LDFLAGS += -ltirpc

define Build/Compile
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS) qcsapi_sockrpc
ifdef CONFIG_PACKAGE_kmod-qtn-pcie2
	$(call Build/Compile/qtn-pcie2)
endif
endef

define Package/qtn-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/libqcsapi_client.so* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qcsapi_sockrpc $(1)/usr/sbin/
	ln -s /tmp/qcsapi_target_ip.conf $(1)/etc
	$(INSTALL_BIN) ./files/qtn-utils.init $(1)/etc/init.d/qtn-utils
endef

define Package/qtn-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Quantenna QV840 (QTS1000) firmware
  DEPENDS:=+dnsmasq
endef

define Package/qtn-firmware/description
  Firmware for Quantenna QV840 (QTS1000) WiFi modules
endef

define Package/qtn-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/qtn
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/u-boot.bin \
		$(PKG_BUILD_DIR)/topaz-linux.lzma.img \
		$(1)/lib/firmware/qtn
	ln -s u-boot.bin $(1)/lib/firmware/qtn/uboot_qsr1000.img
	ln -s topaz-linux.lzma.img $(1)/lib/firmware/qtn/fmac_qsr1000.img
	cd $(1)/lib/firmware && ln -s qtn/u-boot.bin . && ln -s qtn/topaz-linux.lzma.img .
	$(INSTALL_BIN) ./files/qtn-firmware.defaults $(1)/etc/uci-defaults
endef

define Package/qtn-proto
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Quantenna QV840 (QTS1000) netifd support
  DEPENDS:=+qtn-utils
endef

define Package/qtn-proto/install
	echo "######################################### QTN-PROTO INSTALL #####################################"
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DIR) $(1)/lib/netifd
	$(INSTALL_DIR) $(1)/lib/netifd/wireless
	$(INSTALL_BIN) ./files/lib/wifi/quantenna.sh $(1)/lib/wifi
	$(INSTALL_BIN) ./files/lib/netifd/quantenna.sh $(1)/lib/netifd
	$(INSTALL_BIN) ./files/lib/netifd/wireless/quantenna.sh $(1)/lib/netifd/wireless
endef

define KernelPackage/qtn-pcie2
	SECTION:=kernel
	SUBMENU:=Wireless Drivers
	TITLE:=Quantenna QV840 (QTS1000) pcie2 driver
	DEPENDS:=+qtn-utils
	FILES:=$(PKG_BUILD_DIR)/drivers/pcie2/host/arm/qdpc-host.ko
	AUTOLOAD:=$(call AutoProbe,qdpc-host)
endef

define KernelPackage/qtn-pcie2/description
	Quantenna QV840 (QTS1000) WiFi modules PCIE2 driver
endef

define Build/Compile/qtn-pcie2
	echo "######################################### PCIE2 DRIVER BUILD #####################################"
	cd $(PKG_BUILD_DIR)/drivers/pcie2/host/arm
	+$(MAKE) $(CT_MAKEDEFS) $(PKG_JOBS) -C "$(LINUX_DIR)" \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)/drivers/pcie2/host/arm" \
	NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
	modules
endef

$(eval $(call BuildPackage,qtn-utils))
$(eval $(call BuildPackage,qtn-firmware))
$(eval $(call BuildPackage,qtn-proto))
$(eval $(call KernelPackage,qtn-pcie2))

