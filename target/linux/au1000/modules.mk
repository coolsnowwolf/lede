#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/pcmcia-au1000
  SUBMENU:=$(PCMCIA_MENU)
  TITLE:=RMI/AMD Au1000 PCMCIA support
  DEPENDS:=@TARGET_au1000 +kmod-pcmcia-core +kmod-pcmcia-rsrc
  FILES:=$(LINUX_DIR)/drivers/pcmcia/db1xxx_ss.ko
  KCONFIG:=CONFIG_PCMCIA_ALCHEMY_DEVBOARD
  AUTOLOAD:=$(call AutoLoad,41,db1xxx_ss)
endef

$(eval $(call KernelPackage,pcmcia-au1000))
