#!/bin/sh

do_oxnas() {
	. /lib/oxnas.sh

	oxnas_board_detect
}

boot_hook_add preinit_main do_oxnas
