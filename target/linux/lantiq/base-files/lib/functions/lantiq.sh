lantiq_is_vdsl_system() {
	grep -qE "system type.*: (VR9|xRX200)" /proc/cpuinfo
}

lantiq_setup_dsl_helper() {
	local annex="$1"

	ls /lib/modules/$(uname -r)/ltq_atm* 1> /dev/null 2>&1 && \
		ucidef_add_atm_bridge "1" "32" "llc" "bridged" "dsl"

	if lantiq_is_vdsl_system; then
		ucidef_add_vdsl_modem "$annex" "av"
	else
		ucidef_add_adsl_modem "$annex" "/lib/firmware/adsl.bin"
	fi

	ucidef_set_interface_wan "dsl0" "pppoe"
}
