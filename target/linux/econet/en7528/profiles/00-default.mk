# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2025 OpenWrt.org

define Profile/Default
	NAME:=Default Profile
endef

define Profile/Default/Description
	Default package set compatible with most EN7528 boards.
endef
$(eval $(call Profile,Default))
