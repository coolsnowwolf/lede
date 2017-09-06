#!/bin/sh

do_mxs() {
	. /lib/mxs.sh

	mxs_board_detect
}

boot_hook_add preinit_main do_mxs
