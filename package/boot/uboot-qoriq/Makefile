#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
# Copyright (C) 2025 Pawel Dembicki <paweldembicki@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2025.01
PKG_HASH:=cdef7d507c93f1bbd9f015ea9bc21fa074268481405501945abc6f854d5b686f
PKG_RELEASE:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

# U-Boot builds 32-bit PowerPC code with the target compiler's -m32 support.
# Its private libgcc avoids a separate host-specific 32-bit toolchain.
UBOOT_CUSTOMIZE_CONFIG := --enable USE_PRIVATE_LIBGCC

# Build matching bundled dtc and Python bindings instead of the kernel dtc.
DTC:=


define U-Boot/Default
  BUILD_TARGET:=qoriq
  BUILD_SUBTARGET:=generic
  BUILD_DEVICES:=$(1)
  ENV_SIZE:=0x2000
endef

define U-Boot/fsl_T4240RDB-nor
  NAME:=NXP T4240RDB NOR Boot
  BUILD_DEVICES:=fsl_T4240RDB
  UBOOT_CONFIG:=T4240RDB
  UBOOT_IMAGE:=u-boot-dtb.bin
endef

define U-Boot/fsl_T4240RDB-sdboot
  NAME:=NXP T4240RDB SD Card Boot
  BUILD_DEVICES:=fsl_T4240RDB
  UBOOT_CONFIG:=T4240RDB_SDCARD
  UBOOT_IMAGE:=u-boot-with-spl-pbl.bin
endef

UBOOT_TARGETS := \
  fsl_T4240RDB-nor \
  fsl_T4240RDB-sdboot

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.bin
	$(PKG_BUILD_DIR)/tools/mkenvimage -b -s $(ENV_SIZE) \
		-o $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot-env.bin \
		files/$(BUILD_VARIANT)-uEnv.txt
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
