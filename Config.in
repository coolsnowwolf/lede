# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

mainmenu "OpenWrt Configuration"

config MODULES
	option modules
	bool
	default y

config HAVE_DOT_CONFIG
	bool
	default y

source "target/Config.in"

source "config/Config-images.in"

source "config/Config-build.in"

source "config/Config-devel.in"

source "toolchain/Config.in"

source "target/imagebuilder/Config.in"

source "target/sdk/Config.in"

source "target/toolchain/Config.in"

source "tmp/.config-package.in"
