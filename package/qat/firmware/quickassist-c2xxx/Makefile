#
# Copyright (C) DL <revelstone@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quickassist-c2xxx

PKG_VERSION:=1.5
PKG_RELEASE:=2

PKG_SOURCE_VERSION:=1.5.l.1.13.0-19
PKG_SOURCE:=qat$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_URL:=https://01.org/sites/default/files/downloads/intel-quickassist-technology/
PKG_HASH:=afe8339edbbf05099e79c256191584aabf30b564f89a553b9585b2f8e18bee17

PKG_LICENSE:=GPL v3

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=0

define Package/quickassist-c2xxx
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Intel Quick Assist meta-package for c2xxx
  DEPENDS:= \
  	@TARGET_x86_64 \
	@LINUX_4_14 \
	+libopenssl \
	+kmod-crypto-qat-c2xxx \
	+kmod-crypto-qat-c2xxx-usdm
endef

define Package/quickassist-c2xxx/config
  depends on !quickassist-c3xxx-enabled
  config quickassist-c2xxx-enabled
    bool
    default y if PACKAGE_quickassist-c2xxx
    default n
endef

define Package/quickassist-c2xxx/description
 Intel Quick Assist v1.5 utilities and firmware for c2xxx series SoCs
endef

#
# QAT 1.5 kernel driver
#

define KernelPackage/crypto-qat-c2xxx
  SUBMENU:=Cryptographic API modules
  TITLE:=Intel Quick Assist Technology Drivers
  DEPENDS:= \
	@TARGET_x86_64 \
	@LINUX_4_14 \
	+libc \
	+libpthread \
	+libopenssl \
  	+kmod-crypto-manager \
	+kmod-crypto-cbc \
	+kmod-crypto-sha1 \
	+kmod-crypto-sha256 \
	+kmod-crypto-sha512
  FILES:=$(PKG_BUILD_DIR)/build/icp_qa_al.ko
endef

define KernelPackage/crypto-qat-c2xxx/description
 Kernel driver for Intel Quick Assist Technology c2xxx
endef

define KernelPackage/crypto-qat-c2xxx/config
  depends on quickassist-c2xxx-enabled
  source "$(SOURCE)/Config.in"
endef

#
# QAT 1.7 contiguous pinned memory driver
#

define KernelPackage/crypto-qat-c2xxx-usdm
  SUBMENU:=Cryptographic API modules
  TITLE:=Quick Assist Pinned Memory Driver
  DEPENDS:= \
	@TARGET_x86_64 \
	@LINUX_4_14
  FILES:=$(PKG_BUILD_DIR)/build/usdm_drv.ko
  AUTOLOAD:=$(call AutoProbe,usdm_drv)
endef

define KernelPackage/crypto-qat-c2xxx-usdm/config
  depends on quickassist-c2xxx-enabled
endef

define KernelPackage/crypto-qat-c2xxx-usdm/description
 Contiguous pinned memory driver for Intel Quick Assist Technology
endef

ICP_ARCH   = x86_64
MAKE_PATH  = quickassist
MAKE_FLAGS = ARCH_USER="$(ICP_ARCH)"

MAKE_VARS += ICP_ROOT="$(PKG_BUILD_DIR)" \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	KERNEL_SOURCE_ROOT="$(LINUX_DIR)" \
	MACHINE="$(ICP_ARCH)" \
	ICP_BUILD_OUTPUT="$(PKG_BUILD_DIR)/build" \
	ICP_ENV_DIR="$(PKG_BUILD_DIR)/quickassist/build_system/build_files/env_files" \
	ICP_BUILDSYSTEM_PATH="$(PKG_BUILD_DIR)/quickassist/build_system" \
	ICP_TOOLS_TARGET="accelcomp" \
	ICP_NONBLOCKING_PARTIALS_PERFORM="1" \
	ICP_ARCH_USER="$(ICP_ARCH)" \
	LIB_SHARED_FLAGS="-L$(STAGING_DIR)/usr/lib" \
	LD_LIBRARY_PATH="$(PKG_BUILD_DIR)/build"

define Build/Prepare
	(mkdir -p '$(PKG_BUILD_DIR)' && zcat '$(DL_DIR)/$(PKG_SOURCE)' | tar -C '$(PKG_BUILD_DIR)' -xf -)
	$(Build/Patch)
endef

define Build/Compile
	$(call Build/Compile/Default)
	$(call Build/Compile/Default, -C $(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/src/sample_code perf_all)
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

define Package/quickassist/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/share/quickassist/QAT1.5/config
	$(CP) $(PKG_BUILD_DIR)/quickassist/config/* $(1)/usr/share/quickassist/QAT1.5/config
	$(CP) $(PKG_BUILD_DIR)/build/adf_ctl $(1)/usr/sbin/adf_ctl
	$(CP) $(PKG_BUILD_DIR)/quickassist/adf/build/linux_2.6/icp_gige_watchdog $(1)/usr/sbin/icp_gige_watchdog
	$(CP) $(PKG_BUILD_DIR)/build/lib*.{a,so*} $(1)/lib/
	$(CP) $(PKG_BUILD_DIR)/build/*.bin $(1)/lib/firmware/
	$(CP) ./files/qat.init $(1)/etc/init.d/qat
	$(CP) ./files/qat_watchdog.init $(1)/etc/init.d/qat_watchdog
	$(CP) ./files/c2xxx_qa_dev0_single_ae.conf $(1)/etc/c2xxx_qa_dev0.conf
endef

$(eval $(call BuildPackage,quickassist-c2xxx))
$(eval $(call KernelPackage,crypto-qat-c2xxx))
$(eval $(call KernelPackage,crypto-qat-c2xxx-usdm))
