#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=drv_vmmc
PKG_VERSION:=1.9.0
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_HASH:=707f515eb727c032418c4da67d7e86884bb56cdc2a606e8f6ded6057d8767e57
PKG_SOURCE_URL:=http://mirror2.openwrt.org/sources
PKG_MAINTAINER:=John Crispin <john@phrozen.org>

PKG_CHECK_FORMAT_SECURITY:=0
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-vmmc
  SUBMENU:=Voice over IP
  TITLE:=TAPI LL driver for Voice Macro
  URL:=http://www.lantiq.com/
  DEPENDS:=@(TARGET_lantiq_xway||TARGET_lantiq_xrx200) +kmod-ltq-tapi
  FILES:=$(PKG_BUILD_DIR)/src/drv_vmmc.ko
  AUTOLOAD:=$(call AutoProbe,drv_vmmc)
endef

define KernelPackage/ltq-vmmc/description
	Voice Subsystem Low Level Driver for Danube, AR9, VR9 device families
endef

define KernelPackage/ltq-vmmc/config
	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += \
	ARCH=$(LINUX_KARCH) \
	--enable-linux-26 \
	--enable-kernelbuild="$(LINUX_DIR)" \
	--enable-kernelincl="$(LINUX_DIR)/include" \
	--enable-tapiincl="$(STAGING_DIR)/usr/include/drv_tapi" \
	--with-ifxos-incl=$(STAGING_DIR)/usr/include/ifxos \
	$(call autoconf_bool,CONFIG_VOICE_CPE_VMMC_EVENT_LOGGER,el-debug) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_VMMC_PMC,pmc) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_VMMC_DISABLE_DECT_NIBBLE_SWAP,dect-nibble-swap) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_FAX,fax t38) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_CID,cid) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_DECT,dect) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_KPI,kpi) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_LT_GR909,lt calibration) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_HDLC,hdlc) \
	$(call autoconf_bool,CONFIG_VOICE_CPE_TAPI_TRACES,trace)

ifneq ($(CONFIG_VOICE_CPE_VMMC_MPS_HISTORY_SIZE),128)
  CONFIGURE_ARGS += --enable-history-buf=$(CONFIG_VOICE_CPE_VMMC_MPS_HISTORY_SIZE)
endif

#defaults
FW_URL:=http://downloads.openwrt.org/sources/
FW_TARGET:=ifx_firmware.bin
FW_FILE:=fw_voip_ifx.tar.gz
COEF_TARGET:=ifx_bbd_fxs.bin
COEF_FILE:=coef_voip_ifx.tar.gz

FW_DIR:=lib/firmware

FW_TARGET_GENERIC:=$(FW_TARGET)
COEF_TARGET_GENERIC:=$(COEF_TARGET)

ifeq ($(CONFIG_VOICE_CPE_VMMC_WITH_DEVICE_DANUBE)$(CONFIG_LTQ_VOICE_CPE_VMMC_WITH_DEVICE_DANUBE),y)
  CONFIGURE_ARGS += --with-device=DANUBE
  FW_SOURCE:=voip_R12.1.0.1.0-enc.bin
  FW_TARGET:=danube_firmware.bin
  FW_FILE=fw_voip_danube-12.1.0.1.0.tar.gz
  FW_HASH:=bb92a6f1b864f217863e435eab6e5bbf8fe9a06b1398fe5aa8542baf53f7d927
  FW_DOWNLOAD:=1
  COEF_SRC:=danube_bbd_fxs.bin
  COEF_TARGET:=danube_bbd_fxs.bin
  COEF_FILE:=coef_voip_danube-0.9.0.tar.gz
  COEF_HASH:=6578d6c8f4874b7e4bf10d10c7e5b7e913f6a7cdeba998fb04f28c41f94c82c1
  COEF_DOWNLOAD:=1
endif

ifeq ($(CONFIG_VOICE_CPE_VMMC_WITH_DEVICE_AR9),y)
  CONFIGURE_ARGS += --with-device=AR9
  # TODO: add fw/coef
endif

COEF_SRC:=$(COEF_TARGET)

ifeq ($(CONFIG_VOICE_VMMC_WITH_DEVICE_FALCON),y)
  CONFIGURE_ARGS += --with-device=FALCON
  FW_SOURCE:=voip_R1.1.0.6.0-enc.bin
  FW_HASH:=5046a9df736ce302bb240f262c2557c4c9d9b214c22ea67910e3a01a3728c84a
  FW_TARGET:=falcon_voip_fw.bin
  FW_FILE=fw_voip_falcon-1.1.0.6.0.tar.gz
  FW_DOWNLOAD:=1
  COEF_TARGET:=falcon_bbd.bin
# FXS part
ifeq ($(CONFIG_LTQ_VOICE_CPE_VMMC_COEF_FALCON_ETSI),y)
	COEF_SRC:=ETSI_3_10.BIN
endif
ifeq ($(CONFIG_LTQ_VOICE_CPE_VMMC_COEF_FALCON_US600R),y)
	COEF_SRC:=R600_3_10.BIN
endif
ifeq ($(CONFIG_LTQ_VOICE_CPE_VMMC_COEF_FALCON_USE_CUSTOM_FILE),y)
	COEF_SRC:=$(CONFIG_LTQ_VOICE_CPE_VMMC_COEF_FALCON_CUSTOM_FILE)
endif
  COEF_FILE:=coef_voip_falcon.tar.gz
  COEF_HASH:=9b63b2cc7fefdad36d43c307227fc1aade3cf5600ee75fb572da6d908238cf60
  COEF_DOWNLOAD:=1
endif

ifeq ($(CONFIG_VOICE_CPE_VMMC_WITH_DEVICE_VR9),y)
  CONFIGURE_ARGS += --with-device=VR9
  FW_TARGET:=voice_ar9_firmware.bin
  FW_SOURCE:=voip_R2.3.1.7.0-enc.bin
  FW_FILE=fw_voip_vr9-2.3.1.7.0.tar.gz
  FW_HASH:=bc64da5a44a5440d6008f80a5d3f6fdb15c90f9b4023795d1b5b8e35c50bd167
  FW_DOWNLOAD:=1
  COEF_SRC:=ETSI_3_10.BIN
  COEF_TARGET:=vr9_bbd_fxs.bin
  COEF_FILE:=coef_voip_vr9-0.9.3.tar.gz
  COEF_HASH:=3264ed2ab0316bdda419b649fd0d6cadf02540c7e5dc0ea501a7fb680fb371e7
  COEF_DOWNLOAD:=1

  COEF_SRC_FXO:=FXO_ETSI_0_0.BIN
  COEF_TARGET_FXO:=vr9_bbd.bin
endif

define Download/firmware
  FILE:=$(FW_FILE)
  URL:=$(FW_URL)
  HASH:=$(FW_HASH)
endef
$(eval $(if $(FW_DOWNLOAD),$(call Download,firmware)))

define Download/coef
  FILE:=$(COEF_FILE)
  URL:=$(FW_URL)
  HASH:=$(COEF_HASH)
endef
$(eval $(if $(COEF_DOWNLOAD),$(call Download,coef)))

define Build/Configure
	rm -rf \
		$(PKG_BUILD_DIR)/coef \
		$(PKG_BUILD_DIR)/firmware
	mkdir -p \
		$(PKG_BUILD_DIR)/coef \
		$(PKG_BUILD_DIR)/firmware
	$(TAR) -C $(PKG_BUILD_DIR)/firmware -xvzf $(DL_DIR)/$(FW_FILE)
	$(TAR) -C $(PKG_BUILD_DIR)/coef -xvzf $(DL_DIR)/$(COEF_FILE)
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	mkdir -p $(1)/usr/include/drv_vmmc
	$(CP) -v --dereference $(PKG_BUILD_DIR)/include/* $(1)/usr/include/drv_vmmc
	(cd $(1)/usr/include/drv_vmmc && ln -snf . include)
endef

define KernelPackage/ltq-vmmc/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/$(FW_DIR)
	$(INSTALL_BIN) ./files/vmmc.init $(1)/etc/init.d/vmmc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/firmware/$(FW_SOURCE) $(1)/$(FW_DIR)/$(FW_TARGET)
	ln -s /$(FW_DIR)/$(FW_TARGET) $(1)/$(FW_DIR)/$(FW_TARGET_GENERIC)
	$(CP) $(PKG_BUILD_DIR)/coef/$(COEF_SRC) $(1)/$(FW_DIR)/$(COEF_TARGET)
	$(CP) $(PKG_BUILD_DIR)/coef/$(COEF_SRC_FXO) $(1)/$(FW_DIR)/$(COEF_TARGET_FXO)
	ln -s /$(FW_DIR)/$(COEF_TARGET) $(1)/$(FW_DIR)/$(COEF_TARGET_GENERIC)
endef

$(eval $(call KernelPackage,ltq-vmmc))
