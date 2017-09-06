#!/bin/sh

do_brcm63xx() {
	. /lib/brcm63xx.sh

	brcm63xx_detect
}

boot_hook_add preinit_main do_brcm63xx
