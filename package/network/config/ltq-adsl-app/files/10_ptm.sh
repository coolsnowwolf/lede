#!/bin/sh

[ "$DSL_NOTIFICATION_TYPE" = "DSL_STATUS" ] && \
[ "$DSL_TC_LAYER_STATUS" = "EFM" ] && \
! grep -q "ltq_ptm_ar9\|ltq_ptm_ase\|ltq_ptm_danube" /proc/modules || exit 0

logger -p daemon.notice -t "dsl-notify" "Switching to TC-Layer EFM/PTM"

if grep -q "ltq_atm_ar9\|ltq_atm_ase\|ltq_atm_danube" /proc/modules ; then
	logger -p daemon.notice -t "dsl-notify" "Loading EFM/PTM driver while ATM driver is loaded is not possible. Reboot is needed."
	exit
fi

case "$(strings /proc/device-tree/compatible)" in
*lantiq,ar9*)
	soc="ar9"
	;;
*lantiq,ase*)
	soc="ase"
	;;
*lantiq,danube*)
	soc="danube"
	;;
*)
	logger -p daemon.notice -t "dsl-notify" "Unsupported SoC"
	exit
esac

modprobe ltq_ptm_${soc}
