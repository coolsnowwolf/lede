#!/bin/sh

do_adm5120() {
	. /lib/adm5120.sh
}

boot_hook_add preinit_main do_adm5120
