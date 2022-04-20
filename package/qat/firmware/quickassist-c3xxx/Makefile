#
# Copyright (C) Ian Cooper <iancooper@hotmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quickassist-c3xxx

PKG_VERSION:=1.7
PKG_MINOR_VERSION:=L.4.13.0-00009
PKG_RELEASE:=1

PKG_SOURCE:=QAT$(PKG_VERSION).$(PKG_MINOR_VERSION).tar.gz
PKG_SOURCE_URL:=https://downloadmirror.intel.com/30178/eng/
PKG_HASH:=skip

PKG_LICENSE:=GPL v3

PKG_BUILD_DEPENDS:=eudev openssl
QAT_DEFAULT_DEPENDS:= @TARGET_x86_64

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_BUILD_PARALLEL:=0

QAT_BUILD  = $(PKG_BUILD_DIR)/build
QAT_BLDSYS = $(PKG_BUILD_DIR)/quickassist/build_system
QAT_OOT    = $(PKG_BUILD_DIR)/quickassist/qat
QAT_ADF    = $(PKG_BUILD_DIR)/quickassist/utilities/adf_ctl
QAT_ENV    = $(PKG_BUILD_DIR)/quickassist/build_system/build_files/env_files

define Package/quickassist-c3xxx
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Intel Quick Assist Technology for c3xxx SoC
  DEPENDS:= \
	$(QAT_DEFAULT_DEPENDS) \
	$(PKG_DEFAULT_DEPENDS) \
	+libopenssl \
	+libstdcpp \
	+eudev \
	+kmod-itco-wdt \
	+kmod-crypto-manager \
	+kmod-crypto-sha1 \
	+kmod-crypto-sha256 \
	+kmod-crypto-sha512 \
	+kmod-crypto-cbc \
	+kmod-crypto-qat-c3xxx \
	+kmod-crypto-qat-usdm  \
	+kmod-crypto-qat-common \
	+kmod-crypto-qat-api
  USERID:=qat=555:qat=555
endef

define Package/quickassist-c3xxx/description
 Intel Quick Assist v1.7 drivers and utilities for c3xxx series SoCs
endef

define Package/quickassist-c3xxx/config
  source "$(SOURCE)/Config.kernel"
  source "$(SOURCE)/Config.in"
  config quickassist-c3xxx-enabled
    bool
    default y if PACKAGE_quickassist-c3xxx
    default n
endef

define KernelPackage/crypto-qat-c3xxx
  SUBMENU:=Cryptographic API modules
  TITLE:=Intel Quick Assist Technology Driver
  DEPENDS:= \
	$(QAT_DEFAULT_DEPENDS) \
	kmod-crypto-qat-common
  FILES:=$(QAT_OOT)/drivers/crypto/qat/qat_c3xxx/qat_c3xxx.ko
endef

define KernelPackage/crypto-qat-c3xxx/description
 Kernel driver for Intel Quick Assist Technology c3xxx
endef

define KernelPackage/crypto-qat-common
  SUBMENU:=Cryptographic API modules
  TITLE:=Intel Quick Assist Common Driver
  DEPENDS:= \
	$(QAT_DEFAULT_DEPENDS) \
	+kmod-crypto-authenc \
	+kmod-crypto-kpp
  FILES:=$(QAT_OOT)/drivers/crypto/qat/qat_common/intel_qat.ko
endef

define KernelPackage/crypto-qat-common/description
 Kernel driver shared layer for Intel Quick Assist Technology c3xxx
endef

define KernelPackage/crypto-qat-common/config
	depends on quickassist-c3xxx-enabled
endef

define KernelPackage/crypto-qat-api
  SUBMENU:=Cryptographic API modules
  TITLE:=Intel Quick Assist Kernel API
  DEPENDS:= \
	$(QAT_DEFAULT_DEPENDS) \
	kmod-crypto-qat-common
  FILES:=$(QAT_BUILD)/qat_api.ko
endef

define KernelPackage/crypto-qat-api/description
 Kernel API for Intel Quick Assist Technology c3xxx
endef

define KernelPackage/crypto-qat-usdm
  SUBMENU:=Cryptographic API modules
  TITLE:=Intel Quick Assist Pinned Memory Driver
  DEPENDS:= \
	$(QAT_DEFAULT_DEPENDS) \
	kmod-crypto-qat-common
  FILES:=$(QAT_BUILD)/usdm_drv.ko
endef

define KernelPackage/crypto-qat-usdm/description
 Contiguous pinned memory driver for Intel Quick Assist Technology
endef

ICP_ARCH	= x86_64
MAKE_PATH	= quickassist

QAT_CONFIG += $(if $(CONFIG_QAT_DEBUG)                  ,ICP_DEBUG="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_DISABLE_STATS)          ,DISABLE_STATS="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_LOG_SYSLOG)             ,ICP_LOG_SYSLOG="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_PARAM_CHECK)            ,ICP_PARAM_CHECK="y",)
QAT_CONFIG += $(if $(CONFIG_QAT_NONBLOCKING_PARTIALS)   ,ICP_NONBLOCKING_PARTIALS_PERFORM="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_DC_ONLY)                ,ICP_DC_ONLY="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_DC_ONLY)                ,DO_CRYPTO="0",)
QAT_CONFIG += $(if $(CONFIG_QAT_DC_SYM_ONLY)            ,ICP_DC_SYM_ONLY_AM="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_DC_COUNTER_ERROR)       ,ICP_DC_RETURN_COUNTERS_ON_ERROR="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_DISABLE_INLINE)         ,ICP_DISABLE_INLINE="1",)
QAT_CONFIG += $(if $(CONFIG_QAT_HB_FAIL)                ,ICP_HB_FAIL_SYM="y",)
QAT_CONFIG += $(if $(CONFIG_QAT_COEXIST)                ,QAT_COEXISTENCE="y",)
QAT_CONFIG += $(if $(CONFIG_QAT_LKCF)                   ,QAT_NO_LKCF="n",)
QAT_CONFIG += $(if $(CONFIG_QAT_DISABLE_STRICT)         ,CNV_STRICT_MODE="1",)
QAT_CONFIG += $(if $(CONFIG_PKG_CC_STACKPROTECTOR_NONE) ,ICP_DEFENSES_ENABLED="n",ICP_DEFENSES_ENABLED="y")

MAKE_VARS += ICP_ROOT="$(PKG_BUILD_DIR)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CXX="$(TARGET_CXX)" \
	KERNEL_SOURCE_ROOT="$(LINUX_DIR)" \
	KERNEL_SOURCE_DIR="$(QAT_OOT)" \
	KDIR="$(LINUX_DIR)" \
	MACHINE="$(ICP_ARCH)" \
	USE_OPENSSL="1" \
	ICP_BUILD_OUTPUT="$(QAT_BUILD)" \
	ICP_ENV_DIR="$(QAT_ENV)" \
	ICP_BUILDSYSTEM_PATH="$(QAT_BLDSYS)" \
	ICP_TOOLS_TARGET="accelcomp" \
	$(QAT_CONFIG) \
	ICP_KAPI_M="1" \
	ICP_CORE="ia" \
	ICP_OS="linux_2.6" \
	ICP_ARCH_USER="$(ICP_ARCH)" \
	ICP_DEFENSES_ENABLED="n" \
	LIB_SHARED_FLAGS="-L $(STAGING_DIR)/lib -L $(STAGING_DIR)/usr/lib" \
	LD_LIBRARY_PATH="$(PKG_BUILD_DIR)/build" 

#	ICP_DEFENSES_ENABLED="$(ICP_DEFENSES_ENABLED)" \

define Build/Prepare
	(mkdir -p '$(PKG_BUILD_DIR)' && zcat '$(DL_DIR)/$(PKG_SOURCE)' | tar -C '$(PKG_BUILD_DIR)' -xf -)
	$(Build/Patch)
endef

define Build/Configure
	# The shipped configure script is hopelessly broken for cross compilation environments
endef

define Build/Compile
	@echo "QAT Build Configuration: $(QAT_CONFIG)"
	$(call Build/Compile/Default, -C $(QAT_OOT))
	$(call Build/Compile/Default, lac_kernel lac_user)
	$(call Build/Compile/Default, -C $(QAT_ADF))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/icp/quickassist
	$(INSTALL_DIR) $(1)/usr/icp/build
	$(CP) $(PKG_BUILD_DIR)/build/lib*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/quickassist/include/{*.h,dc,lac} $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/include/*.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/quickassist $(1)/usr/icp
	$(CP) $(PKG_BUILD_DIR)/build $(1)/usr/icp
endef

define Package/quickassist-c3xxx/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/hotplug.d
	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/share/quickassist/QAT1.7/config
	$(CP) $(PKG_BUILD_DIR)/quickassist/utilities/adf_ctl/conf_files/c3xxx* $(1)/usr/share/quickassist/QAT1.7/config
	$(CP) $(PKG_BUILD_DIR)/quickassist/utilities/adf_ctl/adf_ctl $(1)/usr/sbin/adf_ctl
	$(CP) $(PKG_BUILD_DIR)/build/*.so* $(1)/lib/
	$(CP) $(PKG_BUILD_DIR)/quickassist/qat/fw/qat_c3xxx* $(1)/lib/firmware/
	$(CP) ./files/qat.init $(1)/etc/init.d/qat
	$(CP) ./files/hotplug.d $(1)/etc
	$(CP) ./files/sysctl.d $(1)/etc
	$(CP) ./files/c3xxx_dev0.conf $(1)/etc/c3xxx_dev0.conf
	$(CP) ./files/c3xxx_dev0.conf.multi $(1)/etc/c3xxx_dev0.conf.multi
endef

define Package/quickassist-c3xxx/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0
[ "$${PKG_UPGRADE}" = "1" ] && exit 0
sysctl -p /etc/sysctl.d/12-hugepages.conf
/etc/init.d/qat enable
exit 0
endef

define Package/quickassist-c3xxx/prerm
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0
/etc/init.d/qat stop
exit 0
endef

$(eval $(call BuildPackage,quickassist-c3xxx))
$(eval $(call KernelPackage,crypto-qat-c3xxx))
$(eval $(call KernelPackage,crypto-qat-common))
$(eval $(call KernelPackage,crypto-qat-api))
$(eval $(call KernelPackage,crypto-qat-usdm))
