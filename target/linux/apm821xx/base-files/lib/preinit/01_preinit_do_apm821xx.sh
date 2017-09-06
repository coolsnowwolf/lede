#!/bin/sh

do_apm821xx() {
	. /lib/apm821xx.sh

	apm821xx_board_detect
}

boot_hook_add preinit_main do_apm821xx
