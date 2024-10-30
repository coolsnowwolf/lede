include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bcm27xx-gpu-fw
PKG_VERSION:=2025.04.30
PKG_VERSION_REAL:=1.$(subst .,,$(PKG_VERSION))
PKG_RELEASE:=1

PKG_SOURCE:=raspi-firmware_$(PKG_VERSION_REAL).orig.tar.xz
PKG_SOURCE_URL:=https://github.com/raspberrypi/firmware/releases/download/$(PKG_VERSION_REAL)
PKG_HASH:=25bcff5992c6d7057de4a5c6834b7f90b7136fe12aa63fa32793329bf74a95bf

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

TAR_OPTIONS+=--strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define Package/bcm27xx-gpu-fw
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_bcm27xx
  TITLE:=bcm27xx-gpu-fw
  DEFAULT:=y if TARGET_bcm27xx
endef

define Package/bcm27xx-gpu-fw/description
  GPU and kernel boot firmware for bcm27xx.
endef

define Build/Compile
	true
endef

define Package/bcm27xx-gpu-fw/install
	true
endef

define Build/InstallDev
	$(CP) $(PKG_BUILD_DIR)/boot/bootcode.bin $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/LICENCE.broadcom $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start_cd.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start_x.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start4.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start4cd.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/start4x.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup_cd.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup_x.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup4.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup4cd.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/boot/fixup4x.dat $(KERNEL_BUILD_DIR)
endef

$(eval $(call BuildPackage,bcm27xx-gpu-fw))
