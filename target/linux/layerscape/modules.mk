#
# Copyright (C) Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ppfe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Freescale PPFE Driver support
  DEPENDS:=@TARGET_layerscape
  KCONFIG:=CONFIG_FSL_PPFE CONFIG_FSL_PPFE_UTIL_DISABLED
  FILES:=$(LINUX_DIR)/drivers/staging/fsl_ppfe/pfe.ko
  AUTOLOAD:=$(call AutoLoad,35,pfe)
endef

define KernelPackage/ppfe/description
 Kernel modules for Freescale PPFE Driver support.
endef

$(eval $(call KernelPackage,ppfe))
