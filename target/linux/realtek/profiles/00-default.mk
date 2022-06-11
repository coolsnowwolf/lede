# SPDX-License-Identifier: GPL-2.0-only

define Profile/Default
	NAME:=Default Profile
	PRIORITY:=1
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef

$(eval $(call Profile,Default))
