#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

BOARDNAME:=Internal Hard-Disk
FEATURES+=targz
DEVICE_TYPE:=nas

define Target/Description
	Build firmware images for Marvell Orion based boards that boot directly from internal disk storage.
	(e.g.: Freecom DataTank 2, ...)
endef
