#!/bin/sh
# travelmate, a wlan connection manager for travel router
# Copyright (c) 2016-2021 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the GNU General Public License v3.

# set (s)hellcheck exceptions
# shellcheck disable=1091,2016,2039,2059,2086,2143,2181,2188

export LC_ALL=C
export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
set -o pipefail

trm_ver="2.0.3"
trm_enabled=0
trm_debug=0
trm_iface=""
trm_captive=1
trm_proactive=1
trm_netcheck=0
trm_autoadd=0
trm_randomize=0
trm_mail=0
trm_vpn=0
trm_mailpgm="/etc/travelmate/travelmate.mail"
trm_vpnpgm="/etc/travelmate/travelmate.vpn"
trm_vpnservice=""
trm_scanbuffer=1024
trm_minquality=35
trm_maxretry=3
trm_maxwait=30
trm_timeout=60
trm_radio=""
trm_connection=""
trm_wpaflags=""
trm_rtfile="/tmp/trm_runtime.json"
trm_wifi="$(command -v wifi)"
trm_fetch="$(command -v curl)"
trm_iwinfo="$(command -v iwinfo)"
trm_logger="$(command -v logger)"
trm_wpa="$(command -v wpa_supplicant)"
trm_captiveurl="http://captive.apple.com"
trm_useragent="Mozilla/5.0 (Linux x86_64; rv:80.0) Gecko/20100101 Firefox/80.0"
trm_ntpfile="/var/state/travelmate.ntp"
trm_vpnfile="/var/state/travelmate.vpn"
trm_mailfile="/var/state/travelmate.mail"
trm_refreshfile="/var/state/travelmate.refresh"
trm_pidfile="/var/run/travelmate.pid"
trm_action="${1:-"start"}"

# load travelmate environment
#
f_env()
{
	local IFS check wpa_checks ubus_check result

	# do nothing on stop
	#
	if [ "${trm_action}" = "stop" ]
	then
		return
	fi

	# (re-)initialize global list variables
	#
	unset trm_stalist trm_radiolist trm_uplinklist trm_wpaflags trm_activesta

	# get system information
	#
	trm_sysver="$(ubus -S call system board 2>/dev/null | jsonfilter -e '@.model' -e '@.release.description' | \
		awk 'BEGIN{ORS=", "}{print $0}' | awk '{print substr($0,1,length($0)-2)}')"

	# check travelmate config
	#
	if [ ! -r "/etc/config/travelmate" ] || [ -z "$(uci -q show travelmate.global.trm_vpn)" ]
	then
		f_log "err" "invalid travelmate config, please re-install the package via opkg with the '--force-reinstall --force-maintainer' options"
	fi

	# load travelmate config
	#
	config_cb()
	{
		local name="${1}" type="${2}"
		if [ "${name}" = "travelmate" ] && [ "${type}" = "global" ]
		then
			option_cb()
			{
				local option="${1}" value="${2}"
				eval "${option}=\"${value}\""
			}
		else
			option_cb()
			{
				return 0
			}
		fi
	}
	config_load travelmate

	# check 'enabled' option
	#
	if [ "${trm_enabled}" != "1" ]
	then
		f_log "info" "travelmate is currently disabled, please set 'trm_enabled' to '1' to use this service"
		/etc/init.d/travelmate stop
	fi

	# check ubus network interface
	#
	if [ -n "${trm_iface}" ]
	then
		ubus_check="$(ubus -t "${trm_maxwait}" wait_for network.wireless network.interface."${trm_iface}" 2>&1)"
		if [ -n "${ubus_check}" ]
		then
			f_log "info" "travelmate interface '${trm_iface}' does not appear on ubus, please check your network setup"
			/etc/init.d/travelmate stop
		fi
	else
		f_log "info" "travelmate is currently not configured, please use the 'Interface Setup' in LuCI or the 'setup' option in CLI"
		/etc/init.d/travelmate stop
	fi

	# check wpa capabilities
	#
	wpa_checks="sae owe eap suiteb192"
	for check in ${wpa_checks}
	do
		if [ -x "${trm_wpa}" ]
		then
			result="$("${trm_wpa}" -v${check} >/dev/null 2>&1; printf "%u" "${?}")"
			if [ -z "${trm_wpaflags}" ]
			then
				if [ "${result}" = "0" ]
				then
					trm_wpaflags="${check}: $(f_char 1)"
				else
					trm_wpaflags="${check}: $(f_char 0)"
				fi
			else
				if [ "${result}" = "0" ]
				then
					trm_wpaflags="$(f_trim "${trm_wpaflags}, ${check}: $(f_char 1)")"
				else
					trm_wpaflags="$(f_trim "${trm_wpaflags}, ${check}: $(f_char 0)")"
				fi
			fi
		fi
	done

	# get and enable wifi devices
	#
	config_load wireless
	config_foreach f_prepdev wifi-device
	if [ -n "$(uci -q changes "wireless")" ]
	then
		uci_commit "wireless"
		f_reconf
	fi

	# load json runtime file
	#
	json_load_file "${trm_rtfile}" >/dev/null 2>&1
	json_select data >/dev/null 2>&1
	if [ "${?}" != "0" ]
	then
		> "${trm_rtfile}"
		json_init
		json_add_object "data"
	fi
	f_log "debug" "f_env     ::: wpa_flags: ${trm_wpaflags}, sys_ver: ${trm_sysver}"
}

# trim helper function
#
f_trim()
{
	local IFS trim="${1}"

	trim="${trim#"${trim%%[![:space:]]*}"}"
	trim="${trim%"${trim##*[![:space:]]}"}"
	printf "%s" "${trim}"
}

# status helper function
#
f_char()
{
	local result input="${1}"

	if [ "${input}" = "1" ]
	then
		result="✔"
	else
		result="✘"
	fi
	printf "%s" "${result}"
}

# wifi reconf helper function
#
f_reconf()
{
	local radio tmp_radio cnt="0"

	"${trm_wifi}" reconf
	for radio in ${trm_radiolist}
	do
		while [ "$(ubus -S call network.wireless status | jsonfilter -l1 -e "@.${radio}.up")" != "true" ]
		do
			if [ "${cnt}" -ge "${trm_maxwait}" ]
			then
				break 2
			fi
			if [ "${radio}" != "${tmp_radio}" ]
			then
				"${trm_wifi}" up "${radio}"
				tmp_radio="${radio}"
			fi
			cnt="$((cnt+1))"
			sleep 1
		done
	done
	f_log "debug" "f_reconf  ::: radio_list: ${trm_radiolist}, radio: ${radio}, cnt: ${cnt}"
}

# vpn helper function
#
f_vpn()
{
	local IFS rc action="${1}"

	if [ "${trm_vpn}" = "1" ] && [ -x "${trm_vpnpgm}" ]
	then
		if [ "${action}" = "disable" ] || { [ "${action}" = "enable" ] && [ ! -f "${trm_vpnfile}" ]; }
		then
			"${trm_vpnpgm}" "${action}" >/dev/null 2>&1
			rc="${?}"
		fi
		if [ "${action}" = "enable" ] && [ "${rc}" = "0" ]
		then
			> "${trm_vpnfile}"
		elif [ "${action}" = "disable" ] && [ -f "${trm_vpnfile}" ]
		then
			rm -f "${trm_vpnfile}"
		fi
	fi
	f_log "debug" "f_vpn     ::: vpn: ${trm_vpn}, vpnservice: ${trm_vpnservice:-"-"}, vpnpgm: ${trm_vpnpgm}, action: ${action}, rc: ${rc:-"-"}"
}

# mac randomizer helper function
#
f_mac()
{
	local result ifname action="${1}" section="${2}"

	if [ "${trm_randomize}" = "1" ] && [ "${action}" = "set" ]
	then
		result="$(hexdump -n6 -ve '/1 "%.02X "' /dev/random 2>/dev/null | \
			awk -v local="2,6,A,E" -v seed="$(date +%s)" 'BEGIN{srand(seed)}NR==1{split(local,b,",");seed=int(rand()*4+1);printf "%s%s:%s:%s:%s:%s:%s",substr($1,0,1),b[seed],$2,$3,$4,$5,$6}')"
		uci_set "wireless" "${section}" "macaddr" "${result}"
	else
		result="$(uci_get "wireless" "${section}" "macaddr")"
		if [ -z "${result}" ]
		then
			ifname="$(ubus -S call network.wireless status 2>/dev/null | jsonfilter -l1 -e '@.*.interfaces[@.config.mode="sta"].ifname')"
			result="$(${trm_iwinfo} "${ifname}" info 2>/dev/null | awk '/Access Point:/{printf "%s",$3}')"
		fi
	fi
	printf "%s" "${result}"
	f_log "debug" "f_mac     ::: action: ${action:-"-"}, section: ${section:-"-"}, mac: ${result:-"-"}"
}

# track/set travelmate connection information
#
f_contrack()
{
	local uplink_config radio_config essid_config bssid_config expiry action="${1}" radio="${2}" essid="${3}" bssid="${4}" cnt=0

	while [ "$(uci_get "travelmate" "@uplink[$cnt]" >/dev/null 2>&1; echo $?)" = "0" ]
	do
		radio_config="$(uci_get "travelmate" "@uplink[$cnt]" "device")"
		essid_config="$(uci_get "travelmate" "@uplink[$cnt]" "ssid")"
		bssid_config="$(uci_get "travelmate" "@uplink[$cnt]" "bssid")"
		if [ "${radio_config}" = "${radio}" ] && [ "${essid_config}" = "${essid}" ] && [ "${bssid_config}" = "${bssid}" ]
		then
			uplink_config="@uplink[$cnt]"
		fi
		cnt="$((cnt+1))"
	done
	if [ -n "${uplink_config}" ]
	then
		case "${action}" in
			"start")
				uci_remove "travelmate" "${uplink_config}" "con_start" 2>/dev/null
				uci_remove "travelmate" "${uplink_config}" "con_end" 2>/dev/null
				if [ -f "${trm_ntpfile}" ]
				then
					uci_set "travelmate" "${uplink_config}" "con_start" "$(date "+%Y.%m.%d-%H:%M:%S")"
				fi
			;;
			"refresh")
				if [ -f "${trm_ntpfile}" ] && [ -z "$(uci_get "travelmate" "${uplink_config}" "con_start")" ]
				then
					uci_set "travelmate" "${uplink_config}" "con_start" "$(date "+%Y.%m.%d-%H:%M:%S")"
				fi
			;;
			"end")
				if [ -f "${trm_ntpfile}" ]
				then
					uci_set "travelmate" "${uplink_config}" "con_end" "$(date "+%Y.%m.%d-%H:%M:%S")"
				fi
			;;
			"start_expiry")
				if [ -f "${trm_ntpfile}" ]
				then
					expiry="$(uci_get "travelmate" "${uplink_config}" "con_start_expiry")"
					uci_set "travelmate" "${uplink_config}" "enabled" "0"
					uci_set "travelmate" "${uplink_config}" "con_end" "$(date "+%Y.%m.%d-%H:%M:%S")"
					f_log "info" "uplink '${radio}/${essid}/${bssid:-"-"}' expired after ${expiry} minutes"
				fi
			;;
			"end_expiry")
				if [ -f "${trm_ntpfile}" ]
				then
					expiry="$(uci_get "travelmate" "${uplink_config}" "con_end_expiry")"
					uci_set "travelmate" "${uplink_config}" "enabled" "1"
					uci_remove "travelmate" "${uplink_config}" "con_start" 2>/dev/null
					uci_remove "travelmate" "${uplink_config}" "con_end" 2>/dev/null
					f_log "info" "uplink '${radio}/${essid}/${bssid:-"-"}' re-enabled after ${expiry} minutes"
				fi
			;;
			"disabled")
				uci_set "travelmate" "${uplink_config}" "enabled" "0"
				if [ -f "${trm_ntpfile}" ]
				then
					uci_set "travelmate" "${uplink_config}" "con_end" "$(date "+%Y.%m.%d-%H:%M:%S")"
				fi
			;;
		esac
		if [ -n "$(uci -q changes "travelmate")" ]
		then
			uci_commit "travelmate"
			if [ ! -f "${trm_refreshfile}" ]
			then
				printf "%s" "cfg_reload" > "${trm_refreshfile}"
			fi
		fi
	fi
}

# get/match travelmate uplink option
#
f_uplink()
{
	local IFS result t_radio t_essid t_bssid t_option="${1}" w_radio="${2}" w_essid="${3}" w_bssid="${4}" cnt=0

	while [ "$(uci_get "travelmate" "@uplink[$cnt]" >/dev/null 2>&1; echo $?)" = "0" ]
	do
		t_radio="$(uci_get "travelmate" "@uplink[$cnt]" "device")"
		t_essid="$(uci_get "travelmate" "@uplink[$cnt]" "ssid")"
		t_bssid="$(uci_get "travelmate" "@uplink[$cnt]" "bssid")"
		if [ -n "${w_radio}" ] && [ -n "${w_essid}" ] && \
			[ "${t_radio}" = "${w_radio}" ] && [ "${t_essid}" = "${w_essid}" ] && [ "${t_bssid}" = "${w_bssid}" ]
		then
			result="$(uci_get "travelmate" "@uplink[$cnt]" "${t_option}")"
			break
		fi
		cnt="$((cnt+1))"
	done
	printf "%s" "${result}"
	f_log "debug" "f_uplink  ::: option: ${t_option}, result: ${result}"
}

# prepare the 'wifi-device' sections
#
f_prepdev()
{
	local IFS disabled radio="${1}"

	disabled="$(uci_get "wireless" "${radio}" "disabled")"
	if [ "${disabled}" = "1" ]
	then
		uci_set wireless "${radio}" disabled 0
	fi

	if [ -z "${trm_radio}" ] && [ -z "$(printf "%s" "${trm_radiolist}" | grep -Fo "${radio}")" ]
	then
		trm_radiolist="$(f_trim "${trm_radiolist} ${radio}")"
	elif [ -n "${trm_radio}" ] && [ -z "${trm_radiolist}" ]
	then
		trm_radiolist="$(f_trim "$(printf "%s" "${trm_radio}" | \
			awk '{while(match(tolower($0),/[a-z0-9_]+/)){ORS=" ";print substr(tolower($0),RSTART,RLENGTH);$0=substr($0,RSTART+RLENGTH)}}')")"
	fi
	f_log "debug" "f_prepdev ::: trm_radio: ${trm_radio:-"-"}, radio: ${radio}, radio_list: ${trm_radiolist:-"-"}, disabled: ${disabled:-"-"}"
}

# add open uplink to new 'wifi-iface' section
#
f_addif()
{
	local IFS uci_cfg offset=1 radio="${1}" essid="${2}"

	config_cb()
	{
		local type="${1}" name="${2}"
		if [ "${type}" = "wifi-iface" ]
		then
			if [ "$(uci -q get "wireless.${name}.ssid")" = "${essid}" ]
			then
				offset=0
			elif [ "${offset}" != "0" ]
			then
				offset="$((offset+1))"
			fi
		fi
		return "${offset}"
	}
	config_load wireless

	if [ "${offset}" != "0" ]
	then
		uci_cfg="trm_uplink${offset}"
		while [ -n "$(uci -q get "wireless.${uci_cfg}")" ]
		do
			offset="$((offset+1))"
			uci_cfg="trm_uplink${offset}"
		done
		uci -q batch <<-EOC
			set wireless."${uci_cfg}"="wifi-iface"
			set wireless."${uci_cfg}".mode="sta"
			set wireless."${uci_cfg}".network="${trm_iface}"
			set wireless."${uci_cfg}".device="${radio}"
			set wireless."${uci_cfg}".ssid="${essid}"
			set wireless."${uci_cfg}".encryption="none"
			set wireless."${uci_cfg}".disabled="1"
		EOC
		uci_cfg="$(uci -q add travelmate uplink)"
		uci -q batch <<-EOC
			set travelmate."${uci_cfg}".device="${radio}"
			set travelmate."${uci_cfg}".ssid="${essid}"
			set travelmate."${uci_cfg}".con_start_expiry="0"
			set travelmate."${uci_cfg}".con_end_expiry="0"
			set travelmate."${uci_cfg}".enabled="1"
		EOC
		if [ -n "$(uci -q changes "travelmate")" ] || [ -n "$(uci -q changes "wireless")" ]
		then
			uci_commit "travelmate"
			uci_commit "wireless"
			f_reconf
			if [ ! -f "${trm_refreshfile}" ]
			then
				printf "%s" "ui_reload" > "${trm_refreshfile}"
			fi
			f_log "info" "open uplink '${radio}/${essid}' added to wireless config"
		fi
	fi
	f_log "debug" "f_addif   ::: radio: ${radio:-"-"}, essid: ${essid}, offset: ${offset:-"-"}"
}

# prepare the 'wifi-iface' sections
#
f_prepif()
{
	local IFS mode radio essid bssid disabled status con_start con_end con_start_expiry con_end_expiry section="${1}" proactive="${2}"

	mode="$(uci_get "wireless" "${section}" "mode")"
	radio="$(uci_get "wireless" "${section}" "device")"
	essid="$(uci_get "wireless" "${section}" "ssid")"
	bssid="$(uci_get "wireless" "${section}" "bssid")"
	disabled="$(uci_get "wireless" "${section}" "disabled")"
	status="$(f_uplink "enabled" "${radio}" "${essid}" "${bssid}")"
	con_start="$(f_uplink "con_start" "${radio}" "${essid}" "${bssid}")"
	con_end="$(f_uplink "con_end" "${radio}" "${essid}" "${bssid}")"
	con_start_expiry="$(f_uplink "con_start_expiry" "${radio}" "${essid}" "${bssid}")"
	con_end_expiry="$(f_uplink "con_end_expiry" "${radio}" "${essid}" "${bssid}")"

	if [ "${status}" = "0" ] && [ -n "${con_end}" ] && [ -n "${con_end_expiry}" ] && [ "${con_end_expiry}" != "0" ]
	then
		d1="$(date -d "${con_end}" "+%s")"
		d2="$(date "+%s")"
		d3="$(((d2-d1)/60))"
		if [ "${d3}" -ge "${con_end_expiry}" ]
		then
			status="1"
			f_contrack "end_expiry" "${radio}" "${essid}" "${bssid}"
		fi
	elif [ "${status}" = "1" ] && [ -n "${con_start}" ] && [ -n "${con_start_expiry}" ] && [ "${con_start_expiry}" != "0" ]
	then
		d1="$(date -d "${con_start}" "+%s")"
		d2="$(date "+%s")"
		d3="$((d1+(con_start_expiry*60)))"
		if [ "${d2}" -gt "${d3}" ]
		then
			status="0"
			f_contrack "start_expiry" "${radio}" "${essid}" "${bssid}"
		fi
	fi

	if [ "${mode}" = "sta" ]
	then
		if [ "${status}" = "0" ] || \
			{ { [ -z "${disabled}" ] || [ "${disabled}" = "0" ]; } && { [ "${proactive}" = "0" ] || [ "${trm_ifstatus}" != "true" ]; } }
		then
			uci_set "wireless" "${section}" "disabled" "1"
		elif [ "${disabled}" = "0" ] && [ "${trm_ifstatus}" = "true" ] && [ "${proactive}" = "1" ]
		then
			if [ -z "${trm_activesta}" ]
			then
				trm_activesta="${section}"
			else
				uci_set "wireless" "${section}" "disabled" "1"
			fi
		fi
		if [ "${status}" = "1" ]
		then
			trm_stalist="$(f_trim "${trm_stalist} ${section}-${radio}")"
		fi
	fi
	f_log "debug" "f_prepif  ::: status: ${status}, section: ${section}, active_sta: ${trm_activesta:-"-"}"
}

# check net status
#
f_net()
{
	local IFS err err_rc err_domain json_raw json_cp json_rc cp_domain result="net nok"

	json_raw="$(${trm_fetch} --user-agent "${trm_useragent}" --referer "http://www.example.com" --write-out "%{json}" --silent --show-error --connect-timeout $((trm_maxwait/10)) "${trm_captiveurl}" 2>/tmp/trm_fetch.err)"
	json_raw="${json_raw#*\{}"
	if [ -s "/tmp/trm_fetch.err" ]
	then
		err="$(awk 'BEGIN{FS="[()'\'' ]"}{printf "%s %s",$3,$(NF-1)}' "/tmp/trm_fetch.err")"
		err_rc="${err% *}"
		err_domain="${err#* }"
		if [ "${err_rc}" = "6" ]
		then
			if [ -n "${err_domain}" ] && [ "${err_domain}" != "timed" ] && [ "${err_domain}" != "${trm_captiveurl#http*://*}" ]
			then
				result="net cp '${err_domain}'"
			fi
		fi
	elif [ -n "${json_raw}" ]
	then
		json_cp="$(printf "%s" "{${json_raw}" | jsonfilter -l1 -e '@.redirect_url' 2>/dev/null)"
		json_rc="$(printf "%s" "{${json_raw}" | jsonfilter -l1 -e '@.response_code' 2>/dev/null)"
		if [ -n "${json_cp}" ]
		then
			cp_domain="${json_cp#http*://*}"
			cp_domain="${cp_domain%%/*}"
			result="net cp '${cp_domain}'"
		else
			if [ "${json_rc}" = "200" ] || [ "${json_rc}" = "204" ]
			then
				result="net ok"
			fi
		fi
	fi
	rm -f "/tmp/trm_fetch.err"
	printf "%s" "${result}"
	f_log "debug" "f_net     ::: fetch: ${trm_fetch}, timeout: $((trm_maxwait/6)), url: ${trm_captiveurl}, user_agent: ${trm_useragent}, result: ${result}, error: ${err:-"-"}"
}

# check interface status
#
f_check()
{
	local IFS ifname radio dev_status result login_script login_script_args cp_domain wait_time="1" enabled="1" mode="${1}" status="${2}" sta_radio="${3}" sta_essid="${4}" sta_bssid="${5}"

	if [ "${mode}" = "initial" ] || [ "${mode}" = "dev" ]
	then
		json_get_var station_id "station_id"
		sta_radio="${station_id%%/*}"
		sta_essid="${station_id%/*}"
		sta_essid="${sta_essid#*/}"
		sta_bssid="${station_id##*/}"
		sta_bssid="${sta_bssid//-/}"
	fi
	if [ "${mode}" != "rev" ] && [ -n "${sta_radio}" ] && [ "${sta_radio}" != "-" ] && [ -n "${sta_essid}" ] && [ "${sta_essid}" != "-" ]
	then
		enabled="$(f_uplink "enabled" "${sta_radio}" "${sta_essid}" "${sta_bssid}")"
	fi
	if { [ "${mode}" != "initial" ] && [ "${mode}" != "dev" ] && [ "${status}" = "false" ]; } || \
		{ [ "${mode}" = "dev" ] && { [ "${status}" = "false" ] || { [ "${trm_ifstatus}" != "${status}" ] && [ "${enabled}" = "0" ]; }; }; }
	then
		f_reconf
	fi
	while [ "${wait_time}" -le "${trm_maxwait}" ]
	do
		dev_status="$(ubus -S call network.wireless status 2>/dev/null)"
		if [ -n "${dev_status}" ]
		then
			if [ "${mode}" = "dev" ]
			then
				if [ "${trm_ifstatus}" != "${status}" ]
				then
					trm_ifstatus="${status}"
					f_jsnup
				fi
				if [ "${status}" = "false" ]
				then
					sleep "$((trm_maxwait/5))"
				fi
				break
			elif [ "${mode}" = "rev" ]
			then
				break
			else
				ifname="$(printf "%s" "${dev_status}" | jsonfilter -l1 -e '@.*.interfaces[@.config.mode="sta"].ifname')"
				if [ -n "${ifname}" ] && [ "${enabled}" = "1" ]
				then
					result="$(f_net)"
					trm_ifquality="$(${trm_iwinfo} "${ifname}" info 2>/dev/null | awk -F '[ ]' '/Link Quality:/{split($NF,var0,"/");printf "%i\n",(var0[1]*100/var0[2])}')"
					if [ "${trm_ifquality}" -ge "${trm_minquality}" ]
					then
						trm_ifstatus="$(ubus -S call network.interface dump 2>/dev/null | jsonfilter -l1 -e "@.interface[@.device=\"${ifname}\"].up")"
						if [ "${trm_ifstatus}" = "true" ]
						then
							if [ "${trm_captive}" = "1" ]
							then
								cp_domain="$(printf "%s" "${result}" | awk -F '['\''| ]' '/^net cp/{printf "%s",$4}')"
								if [ -x "/etc/init.d/dnsmasq" ] && [ -f "/etc/config/dhcp" ] && \
									[ -n "${cp_domain}" ] && [ -z "$(uci_get "dhcp" "@dnsmasq[0]" "rebind_domain" | grep -Fo "${cp_domain}")" ]
								then
									uci_add_list "dhcp" "@dnsmasq[0]" "rebind_domain" "${cp_domain}"
									uci_commit "dhcp"
									/etc/init.d/dnsmasq reload
									f_log "info" "captive portal domain '${cp_domain}' added to to dhcp rebind whitelist"
								fi
								if [ -n "${cp_domain}" ] && [ "${trm_captive}" = "1" ]
								then
									trm_connection="${result:-"-"}/${trm_ifquality}"
									f_jsnup
									login_script="$(f_uplink "script" "${sta_radio}" "${sta_essid}" "${sta_bssid}")"
									if [ -x "${login_script}" ]
									then
										login_script_args="$(f_uplink "script_args" "${sta_radio}" "${sta_essid}" "${sta_bssid}")"
										"${login_script}" ${login_script_args} >/dev/null 2>&1
										rc="${?}"
										f_log "info" "captive portal login '${login_script:0:40} ${login_script_args:0:20}' for '${cp_domain}' has been executed with rc '${rc}'"
										if [ "${rc}" = "0" ]
										then
											result="$(f_net)"
										fi
									fi
								fi
							fi
							if [ "${trm_netcheck}" = "1" ] && [ "${result}" = "net nok" ]
							then
								f_log "info" "uplink has no internet (new connection)"
								f_vpn "disable"
								trm_ifstatus="${status}"
								f_jsnup
								break
							fi
							trm_connection="${result:-"-"}/${trm_ifquality}"
							f_jsnup
							break
						fi
					elif [ -n "${trm_connection}" ]
					then
						if [ "${trm_ifquality}" -lt "${trm_minquality}" ]
						then
							f_log "info" "uplink is out of range (${trm_ifquality}/${trm_minquality})"
							f_vpn "disable"
							unset trm_connection
							trm_ifstatus="${status}"
							f_contrack "end" "${sta_radio}" "${sta_essid}" "${sta_bssid}"
						elif [ "${trm_netcheck}" = "1" ] && [ "${result}" = "net nok" ]
						then
							f_log "info" "uplink has no internet (existing connection)"
							f_vpn "disable"
							unset trm_connection
							trm_ifstatus="${status}"
						fi
						f_jsnup
						break
					elif [ "${mode}" = "initial" ]
					then
						trm_ifstatus="${status}"
						f_jsnup
						break
					fi
				elif [ -n "${trm_connection}" ]
				then
					f_vpn "disable"
					unset trm_connection
					trm_ifstatus="${status}"
					f_jsnup
					break
				elif [ "${mode}" = "initial" ]
				then
					trm_ifstatus="${status}"
					f_jsnup
					break
				fi
			fi
		fi
		if [ "${mode}" = "initial" ]
		then
			trm_ifstatus="${status}"
			f_jsnup
			break
		fi
		wait_time="$((wait_time+1))"
		sleep 1
	done
	f_log "debug" "f_check   ::: mode: ${mode}, name: ${ifname:-"-"}, status: ${trm_ifstatus}, enabled: ${enabled}, connection: ${trm_connection:-"-"}, wait: ${wait_time}, max_wait: ${trm_maxwait}, min_quality: ${trm_minquality}, captive: ${trm_captive}, netcheck: ${trm_netcheck}"
}

# update runtime information
#
f_jsnup()
{
	local IFS section last_date last_station sta_iface sta_radio sta_essid sta_bssid sta_mac dev_status last_status status="${trm_ifstatus}" ntp_done="0" vpn_done="0" mail_done="0"

	if [ "${status}" = "true" ]
	then
		status="connected (${trm_connection:-"-"})"
		dev_status="$(ubus -S call network.wireless status 2>/dev/null)"
		if [ -n "${dev_status}" ]
		then
			section="$(printf "%s" "${dev_status}" | jsonfilter -l1 -e '@.*.interfaces[@.config.mode="sta"].section')"
			if [ -n "${section}" ]
			then
				sta_iface="$(uci_get "wireless" "${section}" "network")"
				sta_radio="$(uci_get "wireless" "${section}" "device")"
				sta_essid="$(uci_get "wireless" "${section}" "ssid")"
				sta_bssid="$(uci_get "wireless" "${section}" "bssid")"
				sta_mac="$(f_mac "get" "${section}")"
			fi
		fi
		json_get_var last_date "last_run"
		json_get_var last_station "station_id"
		json_get_var last_status "travelmate_status"

		if { [ -f "${trm_ntpfile}" ] && [ ! -s "${trm_ntpfile}" ]; } || [ "${last_status}" = "running (not connected)" ] || \
			{ [ -n "${last_station}" ] && [ "${last_station}" != "${sta_radio:-"-"}/${sta_essid:-"-"}/${sta_bssid:-"-"}" ]; }
		then
			last_date="$(date "+%Y.%m.%d-%H:%M:%S")"
			if [ -f "${trm_ntpfile}" ] && [ ! -s "${trm_ntpfile}" ]
			then
				printf "%s" "${last_date}" > "${trm_ntpfile}"
			fi
		fi
	elif [ "${status}" = "error" ]
	then
		unset trm_connection
		status="program error"
	else
		unset trm_connection
		status="running (not connected)"
	fi

	if [ -z "${last_date}" ]
	then
		last_date="$(date "+%Y.%m.%d-%H:%M:%S")"
	fi
	if [ -s "${trm_ntpfile}" ]
	then
		ntp_done="1"
	fi
	if [ "${trm_vpn}" = "1" ] && [ -f "${trm_vpnfile}" ]
	then
		vpn_done="1"
	fi
	if [ "${trm_mail}" = "1" ] && [ -f "${trm_mailfile}" ]
	then
		mail_done="1"
	fi
	json_add_string "travelmate_status" "${status}"
	json_add_string "travelmate_version" "${trm_ver}"
	json_add_string "station_id" "${sta_radio:-"-"}/${sta_essid:-"-"}/${sta_bssid:-"-"}"
	json_add_string "station_mac" "${sta_mac:-"-"}"
	json_add_string "station_interface" "${sta_iface:-"-"}"
	json_add_string "wpa_flags" "${trm_wpaflags:-"-"}"
	json_add_string "run_flags" "captive: $(f_char ${trm_captive}), proactive: $(f_char ${trm_proactive}), netcheck: $(f_char ${trm_netcheck}), autoadd: $(f_char ${trm_autoadd}), randomize: $(f_char ${trm_randomize})"
	json_add_string "ext_hooks" "ntp: $(f_char ${ntp_done}), vpn: $(f_char ${vpn_done}), mail: $(f_char ${mail_done})"
	json_add_string "last_run" "${last_date}"
	json_add_string "system" "${trm_sysver}"
	json_dump > "${trm_rtfile}"

	if [ "${status%% (net ok/*}" = "connected" ]
	then
		f_vpn "enable"
		if [ "${trm_mail}" = "1" ] && [ -x "${trm_mailpgm}" ] && [ "${ntp_done}" = "1" ] && [ "${mail_done}" = "0" ]
		then
			if [ "${trm_vpn}" = "0" ] || [ "${vpn_done}" = "1" ]
			then
				> "${trm_mailfile}"
				"${trm_mailpgm}" >/dev/null 2>&1
			fi
		fi
	else
		f_vpn "disable"
	fi
	f_log "debug" "f_jsnup   ::: section: ${section:-"-"}, status: ${status:-"-"}, sta_iface: ${sta_iface:-"-"}, sta_radio: ${sta_radio:-"-"}, sta_essid: ${sta_essid:-"-"}, sta_bssid: ${sta_bssid:-"-"}, ntp: ${ntp_done}, vpn: ${trm_vpn}/${vpn_done}, mail: ${trm_mail}/${mail_done}"
}

# write to syslog
#
f_log()
{
	local IFS class="${1}" log_msg="${2}"

	if [ -n "${log_msg}" ] && { [ "${class}" != "debug" ] || [ "${trm_debug}" = "1" ]; }
	then
		if [ -x "${trm_logger}" ]
		then
			"${trm_logger}" -p "${class}" -t "trm-${trm_ver}[${$}]" "${log_msg}"
		else
			printf "%s %s %s\\n" "${class}" "trm-${trm_ver}[${$}]" "${log_msg}"
		fi
		if [ "${class}" = "err" ]
		then
			trm_ifstatus="error"
			f_jsnup
			> "${trm_pidfile}"
			exit 1
		fi
	fi
}

# main function for connection handling
#
f_main()
{
	local IFS cnt retrycnt spec scan_dev scan_list scan_essid scan_bssid scan_open scan_quality
	local station_id section sta sta_essid sta_bssid sta_radio sta_iface sta_mac config_essid config_bssid config_radio

	f_check "initial" "false"
	f_log "debug" "f_main    ::: status: ${trm_ifstatus}, proactive: ${trm_proactive}"
	if [ "${trm_ifstatus}" != "true" ] || [ "${trm_proactive}" = "1" ]
	then
		config_load wireless
		config_foreach f_prepif wifi-iface ${trm_proactive}
		if [ "${trm_ifstatus}" = "true" ] && [ -n "${trm_activesta}" ] && [ "${trm_proactive}" = "1" ]
		then
			json_get_var station_id "station_id"
			config_radio="${station_id%%/*}"
			config_essid="${station_id%/*}"
			config_essid="${config_essid#*/}"
			config_bssid="${station_id##*/}"
			config_bssid="${config_bssid//-/}"
			f_check "dev" "true"
			f_log "debug" "f_main    ::: config_radio: ${config_radio}, config_essid: \"${config_essid}\", config_bssid: ${config_bssid:-"-"}"
		else
			uci_commit "wireless"
			f_check "dev" "false"
		fi
		f_log "debug" "f_main    ::: radio_list: ${trm_radiolist}, sta_list: ${trm_stalist:0:${trm_scanbuffer}}"

		# radio loop
		#
		for radio in ${trm_radiolist}
		do
			if [ -z "$(printf "%s" "${trm_stalist}" | grep -o "\\-${radio}")" ]
			then
				f_log "info" "no station on radio '${radio}'"
				continue
			fi

			# station loop
			#
			for sta in ${trm_stalist}
			do
				section="${sta%%-*}"
				sta_radio="$(uci_get "wireless" "${section}" "device")"
				sta_essid="$(uci_get "wireless" "${section}" "ssid")"
				sta_bssid="$(uci_get "wireless" "${section}" "bssid")"
				sta_iface="$(uci_get "wireless" "${section}" "network")"
				sta_mac="$(f_mac "get" "${section}")"
				if [ -z "${sta_radio}" ] || [ -z "${sta_essid}" ] || [ -z "${sta_iface}" ]
				then
					f_log "info" "invalid wireless section '${section}'"
					continue
				fi
				if [ "${sta_radio}" = "${config_radio}" ] && [ "${sta_essid}" = "${config_essid}" ] && [ "${sta_bssid}" = "${config_bssid}" ]
				then
					f_contrack "refresh" "${config_radio}" "${config_essid}" "${config_bssid}"
					f_log "info" "uplink still in range '${config_radio}/${config_essid}/${config_bssid:-"-"}' with mac '${sta_mac:-"-"}'"
					break 2
				fi
				f_log "debug" "f_main    ::: sta_radio: ${sta_radio}, sta_essid: \"${sta_essid}\", sta_bssid: ${sta_bssid:-"-"}"
				if [ -z "${scan_list}" ]
				then
					scan_dev="$(ubus -S call network.wireless status 2>/dev/null | jsonfilter -l1 -e "@.${radio}.interfaces[0].ifname")"
					scan_list="$("${trm_iwinfo}" "${scan_dev:-${radio}}" scan 2>/dev/null | \
						awk 'BEGIN{FS="[[:space:]]"}/Address:/{var1=$NF}/ESSID:/{var2="";for(i=12;i<=NF;i++)if(var2==""){var2=$i}else{var2=var2" "$i};
						gsub(/,/,".",var2)}/Quality:/{split($NF,var0,"/")}/Encryption:/{if($NF=="none"){var3="+"}else{var3="-"};printf "%i,%s,%s,%s\n",(var0[1]*100/var0[2]),var1,var2,var3}' | \
						sort -rn | awk -v buf="${trm_scanbuffer}" 'BEGIN{ORS=","}{print substr($0,1,buf)}')"
					f_log "debug" "f_main    ::: radio: ${radio}, scan_device: ${scan_dev}, scan_buffer: ${trm_scanbuffer}, scan_list: ${scan_list:-"-"}"
					if [ -z "${scan_list}" ]
					then
						f_log "info" "no scan results on '${radio}'"
						continue 2
					fi
				fi

				# scan loop
				#
				IFS=","
				for spec in ${scan_list}
				do
					if [ -z "${scan_quality}" ]
					then
						scan_quality="${spec}"
					elif [ -z "${scan_bssid}" ]
					then
						scan_bssid="${spec}"
					elif [ -z "${scan_essid}" ]
					then
						scan_essid="${spec}"
					elif [ -z "${scan_open}" ]
					then
						scan_open="${spec}"
					fi
					if [ -n "${scan_quality}" ] && [ -n "${scan_bssid}" ] && [ -n "${scan_essid}" ] && [ -n "${scan_open}" ]
					then
						if [ "${scan_quality}" -ge "${trm_minquality}" ]
						then
							if { { [ "${scan_essid}" = "\"${sta_essid//,/.}\"" ] && { [ -z "${sta_bssid}" ] || [ "${scan_bssid}" = "${sta_bssid}" ]; } } || \
								{ [ "${scan_bssid}" = "${sta_bssid}" ] && [ "${scan_essid}" = "unknown" ]; } } && [ "${radio}" = "${sta_radio}" ]
							then
								f_vpn "disable"
								f_log "debug" "f_main    ::: scan_quality: ${scan_quality}, scan_essid: ${scan_essid}, scan_bssid: ${scan_bssid:-"-"}, scan_open: ${scan_open}"
								if [ -n "${config_radio}" ]
								then
									uci_set "wireless" "${trm_activesta}" "disabled" "1"
									uci_commit "wireless"
									f_contrack "end" "${config_radio}" "${config_essid}" "${config_bssid}"
									f_log "info" "uplink connection terminated '${config_radio}/${config_essid}/${config_bssid:-"-"}'"
									unset trm_connection config_radio config_essid config_bssid
								fi

								# retry loop
								#
								retrycnt=1
								trm_radio="${sta_radio}"
								while [ "${retrycnt}" -le "${trm_maxretry}" ]
								do
									if [ "${trm_randomize}" = "1" ]
									then
										sta_mac="$(f_mac "set" "${section}")"
									fi
									uci_set "wireless" "${section}" "disabled" "0"
									f_check "sta" "false" "${sta_radio}" "${sta_essid}" "${sta_bssid}"
									if [ "${trm_ifstatus}" = "true" ]
									then
										unset IFS scan_list
										rm -f "${trm_mailfile}"
										uci_commit "wireless"
										f_contrack "start" "${sta_radio}" "${sta_essid}" "${sta_bssid}"
										if [ "${trm_randomize}" = "0" ]
										then
											sta_mac="$(f_mac "get" "${section}")"
										fi
										f_log "info" "connected to uplink '${sta_radio}/${sta_essid}/${sta_bssid:-"-"}' with mac '${sta_mac:-"-"}' (${retrycnt}/${trm_maxretry})"
										return 0
									else
										uci -q revert "wireless"
										f_check "rev" "false"
										if [ "${retrycnt}" = "${trm_maxretry}" ]
										then
											f_contrack "disabled" "${sta_radio}" "${sta_essid}" "${sta_bssid}"
											f_log "info" "uplink has been disabled '${sta_radio}/${sta_essid}/${sta_bssid:-"-"}' (${retrycnt}/${trm_maxretry})"
											break 2
										else
											f_jsnup
											f_log "info" "can't connect to uplink '${sta_radio}/${sta_essid}/${sta_bssid:-"-"}' (${retrycnt}/${trm_maxretry})"
										fi
									fi
									retrycnt="$((retrycnt+1))"
									sleep "$((trm_maxwait/6))"
								done
							elif [ "${trm_autoadd}" = "1" ] && [ "${scan_open}" = "+" ] && [ "${scan_essid}" != "unknown" ]
							then
								scan_essid="${scan_essid%?}"
								scan_essid="${scan_essid:1}"
								f_addif "${sta_radio}" "${scan_essid}"
							fi
							unset scan_quality scan_bssid scan_essid scan_open
							continue
						else
							unset scan_quality scan_bssid scan_essid scan_open
							continue
						fi
					fi
				done
				unset IFS scan_quality scan_bssid scan_essid scan_open
			done
			unset scan_list
		done
	fi
}

# source required system libraries
#
if [ -r "/lib/functions.sh" ] && [ -r "/usr/share/libubox/jshn.sh" ]
then
	. "/lib/functions.sh"
	. "/usr/share/libubox/jshn.sh"
else
	f_log "err" "system libraries not found"
fi

# control travelmate actions
#
if [ "${trm_action}" != "stop" ]
then
	f_env
fi
while true
do
	if [ -z "${trm_action}" ]
	then
		rc=0
		while true
		do
			if [ "${rc}" = "0" ]
			then
				f_check "initial" "false"
			fi
			sleep "${trm_timeout}" 0
			rc=${?}
			if [ "${rc}" != "0" ]
			then
				f_check "initial" "false"
			fi
			if [ "${rc}" = "0" ] || { [ "${rc}" != "0" ] && [ "${trm_ifstatus}" = "false" ]; }
			then
				break
			fi
		done
	elif [ "${trm_action}" = "stop" ]
	then
		if [ -s "${trm_pidfile}" ]
		then
			f_log "info" "travelmate instance stopped ::: action: ${trm_action}, pid: $(cat ${trm_pidfile} 2>/dev/null)"
			> "${trm_rtfile}"
			> "${trm_pidfile}"
		fi
		break
	else
		f_log "info" "travelmate instance started ::: action: ${trm_action}, pid: ${$}"
	fi
	json_cleanup
	f_env
	f_main
	unset trm_action
done
