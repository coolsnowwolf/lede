#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ARCH:=aarch64
BOARDNAME:=layerscape 64b boards

define Target/Description
	Build firmware images for $(BOARDNAME) SoC devices.
endef

