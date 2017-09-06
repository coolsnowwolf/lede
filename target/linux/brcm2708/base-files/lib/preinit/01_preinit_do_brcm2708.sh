#!/bin/sh
# Copyright (C) 2015 OpenWrt.org

do_brcm2708() {
	. /lib/brcm2708.sh

	brcm2708_detect
}

boot_hook_add preinit_main do_brcm2708
