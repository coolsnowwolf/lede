#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ata-rb532-cf
  SUBMENU:=$(BLOCK_MENU)
  TITLE:=RB532 Compact Flash support
  DEPENDS:=@TARGET_rb532 @BROKEN
  KCONFIG:= \
  	CONFIG_PATA_PLATFORM \
  	CONFIG_PATA_RB532
  FILES:=\
  	$(LINUX_DIR)/drivers/ata/pata_platform.ko \
  	$(LINUX_DIR)/drivers/ata/pata_rb532_cf.ko
  AUTOLOAD:=$(call AutoLoad,41,pata_platform pata_rb532_cf,1)
  $(call AddDepends/ata)
endef

define KernelPackage/ata-rb532-cf/description
  RB532 Compact Flash support.
endef

$(eval $(call KernelPackage,ata-rb532-cf))


define KernelPackage/input-rb532
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RB532 button device support
  DEPENDS:=@TARGET_rb532 +kmod-input-polldev
  KCONFIG:= \
	CONFIG_INPUT_MISC=y \
	CONFIG_INPUT_RB532_BUTTON
  FILES:=$(LINUX_DIR)/drivers/input/misc/rb532_button.ko
  AUTOLOAD:=$(call AutoLoad,62,rb532_button)
  $(call AddDepends/input)
endef

define KernelPackage/input-rb532/description
  Kernel module for RB532 button
endef

$(eval $(call KernelPackage,input-rb532))
