#!/bin/sh

do_ar71xx() {
	. /lib/ar71xx.sh

	ar71xx_board_detect
}

boot_hook_add preinit_main do_ar71xx
