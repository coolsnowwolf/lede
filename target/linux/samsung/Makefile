#
# Copyright (C) 2018 Jianhui Zhao <jianhuizhao329@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=samsung
BOARDNAME:=Samsung s3c24xx/s5pv210
SUBTARGETS:=s5pv210

KERNEL_PATCHVER:=4.14

define Target/Description
	Build firmware images for Samsung s3c24xx/s5pv210 based boards.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

$(eval $(call BuildTarget))
