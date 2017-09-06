#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

FIREWIRE_MENU:=FireWire support

define KernelPackage/firewire
  SUBMENU:=$(FIREWIRE_MENU)
  TITLE:=Support for FireWire (new stack)
  DEPENDS:=@PCI_SUPPORT +kmod-lib-crc-itu-t
  KCONFIG:=CONFIG_FIREWIRE
  FILES:=$(LINUX_DIR)/drivers/firewire/firewire-core.ko
endef

define KernelPackage/firewire/description
 Kernel support for FireWire (new stack)
endef

$(eval $(call KernelPackage,firewire))


define KernelPackage/firewire-ohci
  SUBMENU:=$(FIREWIRE_MENU)
  TITLE:=Support for OHCI-1394 controllers
  DEPENDS:=kmod-firewire
  KCONFIG:= \
	CONFIG_FIREWIRE_OHCI \
	CONFIG_FIREWIRE_OHCI_DEBUG=n \
	CONFIG_FIREWIRE_OHCI_REMOTE_DMA=n
  FILES:=$(LINUX_DIR)/drivers/firewire/firewire-ohci.ko
  AUTOLOAD:=$(call AutoProbe,firewire-ohci)
endef


define KernelPackage/firewire-ohci/description
 Kernel support for FireWire OHCI-1394 controllers
endef

$(eval $(call KernelPackage,firewire-ohci))


define KernelPackage/firewire-sbp2
  SUBMENU:=$(FIREWIRE_MENU)
  TITLE:=Support for SBP-2 devices over FireWire
  DEPENDS:=kmod-firewire +kmod-scsi-core
  KCONFIG:=CONFIG_FIREWIRE_SBP2
  FILES:=$(LINUX_DIR)/drivers/firewire/firewire-sbp2.ko
  AUTOLOAD:=$(call AutoProbe,firewire-sbp2)
endef

define KernelPackage/firewire-sbp2/description
 Kernel support for SBP-2 devices over FireWire
endef

$(eval $(call KernelPackage,firewire-sbp2))


define KernelPackage/firewire-net
  SUBMENU:=$(FIREWIRE_MENU)
  TITLE:=Support for IP networking over FireWire
  DEPENDS:=kmod-firewire
  KCONFIG:=CONFIG_FIREWIRE_NET
  FILES:=$(LINUX_DIR)/drivers/firewire/firewire-net.ko
  AUTOLOAD:=$(call AutoProbe,firewire-net)
endef

define KernelPackage/firewire-net/description
 Kernel support for IPv4 over FireWire
endef

$(eval $(call KernelPackage,firewire-net))
