#!/bin/sh

do_arc() {
        . /lib/arc.sh

        arc_board_detect
}

boot_hook_add preinit_main do_arc
