#!/bin/sh

[ "$DSL_NOTIFICATION_TYPE" = "DSL_STATUS" ] && \
[ "$DSL_TC_LAYER_STATUS" = "ATM" ] && \
! grep -q "ltq_atm_vr9" /proc/modules || exit 0

logger -p daemon.notice -t "dsl-notify" "Switching to TC-Layer ATM"

if grep -q "ltq_ptm_vr9" /proc/modules ; then
	logger -p daemon.notice -t "dsl-notify" "Loading ATM driver while EFM/PTM driver is loaded is not possible. Reboot is needed."
	exit
fi

modprobe ltq_atm_vr9
