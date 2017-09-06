#
# Copyright (C) Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ppfe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Freescale PPFE Driver support
  KCONFIG:=CONFIG_FSL_PPFE
  FILES:=$(LINUX_DIR)/drivers/staging/fsl_ppfe/pfe.ko
  AUTOLOAD:=$(call AutoLoad,35,ppfe)
endef

define KernelPackage/ppfe/description
 Kernel modules for Freescale PPFE Driver support.
endef

$(eval $(call KernelPackage,ppfe))
