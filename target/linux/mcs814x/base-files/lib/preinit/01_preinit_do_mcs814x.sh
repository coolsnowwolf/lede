#!/bin/sh

do_mcs814x() {
	. /lib/mcs814x.sh

	mcs814x_board_detect
}

boot_hook_add preinit_main do_mcs814x
