#!/bin/sh

do_at91() {
	. /lib/at91.sh

	at91_board_detect
}

boot_hook_add preinit_main do_at91
