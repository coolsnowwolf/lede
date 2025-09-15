# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) Jiang Yutang <jiangyutang1978@gmail.com>

define KernelPackage/ahci-qoriq
  SUBMENU:=$(BLOCK_MENU)
  TITLE:=Freescale QorIQ AHCI SATA support
  KCONFIG:=CONFIG_AHCI_QORIQ
  FILES:=$(LINUX_DIR)/drivers/ata/ahci_qoriq.ko
  AUTOLOAD:=$(call AutoLoad,40,ahci-qoriq,1)
  $(call AddDepends/ata,+kmod-ata-ahci-platform @TARGET_layerscape)
endef

define KernelPackage/ahci-qoriq/description
 This option enables support for the Freescale QorIQ AHCI SoC's
 onboard AHCI SATA.
endef

$(eval $(call KernelPackage,ahci-qoriq))

define KernelPackage/ppfe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Freescale PPFE Driver support
  DEPENDS:=@TARGET_layerscape
  KCONFIG:=CONFIG_FSL_PPFE \
  	CONFIG_FSL_PPFE_UTIL_DISABLED=y
  FILES:=$(LINUX_DIR)/drivers/staging/fsl_ppfe/pfe.ko
  AUTOLOAD:=$(call AutoLoad,35,pfe)
endef

define KernelPackage/ppfe/description
 Kernel modules for Freescale PPFE Driver support.
endef

$(eval $(call KernelPackage,ppfe))
