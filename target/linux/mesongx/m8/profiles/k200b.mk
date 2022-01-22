#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/k200b
    NAME:= meson8 k200b
    PACKAGES:= 
endef

k200b_KERNELDTS:="meson8_k200b_1G_emmc_sdio"

define Profile/k200b/Description
	Package set optimized for k200b
endef

$(eval $(call Profile,k200b))
