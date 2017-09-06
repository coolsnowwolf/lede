#!/bin/sh

do_mpc85xx() {
	. /lib/mpc85xx.sh

	mpc85xx_board_detect
}

boot_hook_add preinit_main do_mpc85xx
