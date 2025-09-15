#!/bin/sh

[ "$DSL_NOTIFICATION_TYPE" = "DSL_STATUS" ] && \
[ "$DSL_TC_LAYER_STATUS" = "EFM" ] && \
! grep -q "ltq_ptm_vr9" /proc/modules || exit 0

logger -p daemon.notice -t "dsl-notify" "Switching to TC-Layer EFM/PTM"

if grep -q "ltq_atm_vr9" /proc/modules ; then
	logger -p daemon.notice -t "dsl-notify" "Loading EFM/PTM driver while ATM driver is loaded is not possible. Reboot is needed."
	exit
fi

modprobe ltq_ptm_vr9
