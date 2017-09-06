# 
# Copyright (C) 2006-2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

export TARGET_BUILD=1

prereq clean download prepare compile install menuconfig nconfig oldconfig update refresh: FORCE
	@+$(NO_TRACE_MAKE) -C $(BOARD) $@
