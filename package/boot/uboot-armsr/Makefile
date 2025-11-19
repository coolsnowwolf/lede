include $(TOPDIR)/rules.mk

PKG_VERSION:=2025.04
PKG_RELEASE:=1

PKG_HASH:=439d3bef296effd54130be6a731c5b118be7fddd7fcc663ccbc5fb18294d8718

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=armsr
endef

define U-Boot/qemu_armv7
  NAME:=QEMU ARM Virtual Machine 32-bit
  BUILD_SUBTARGET:=armv7
  BUILD_DEVICES:=generic
  UBOOT_CONFIG:=qemu_arm
endef

define U-Boot/qemu_armv8
  NAME:=QEMU ARM Virtual Machine 64-bit
  BUILD_SUBTARGET:=armv8
  BUILD_DEVICES:=generic
  UBOOT_CONFIG:=qemu_arm64
endef

UBOOT_TARGETS := \
	qemu_armv7 \
	qemu_armv8

# mkeficapsule host tool is disabled
# due to a host-side GnuTLS dependency
UBOOT_CUSTOMIZE_CONFIG := \
	--enable CMD_EFIDEBUG \
	--disable TOOLS_MKEFICAPSULE

$(eval $(call BuildPackage/U-Boot))
