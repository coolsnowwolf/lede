#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/k200
    NAME:= meson8 k200
    PACKAGES:= 
endef

k200_KERNELDTS:="meson8_k200b_1G_emmc_sdio"

define Profile/k200/Description
	Package set optimized for k200
endef

$(eval $(call Profile,k200))
