#!/bin/sh

do_ramips() {
	. /lib/ramips.sh

	ramips_board_detect
}

boot_hook_add preinit_main do_ramips
