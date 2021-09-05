#!/bin/sh
# banIP - ban incoming and outgoing ip adresses/subnets via ipset
# written by Dirk Brenken (dev@brenken.org)
#
# This is free software, licensed under the GNU General Public License v3.
#
# (s)hellcheck exceptions
# shellcheck disable=1091,2030,2031,2034,2039,2086,2129,2140,2143,2154,2181,2183,2188

# set initial defaults
#
export LC_ALL=C
export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
set -o pipefail
ban_ver="0.7.9"
ban_enabled="0"
ban_mail_enabled="0"
ban_proto4_enabled="0"
ban_proto6_enabled="0"
ban_logsrc_enabled="0"
ban_logdst_enabled="0"
ban_monitor_enabled="0"
ban_autodetect="1"
ban_autoblacklist="1"
ban_autowhitelist="1"
ban_whitelistonly="0"
ban_logterms=""
ban_loglimit="100"
ban_ssh_logcount="3"
ban_luci_logcount="3"
ban_nginx_logcount="5"
ban_mailactions=""
ban_search=""
ban_devs=""
ban_ifaces=""
ban_debug="0"
ban_maxqueue="4"
ban_fetchutil=""
ban_ip_cmd="$(command -v ip)"
ban_ipt4_cmd="$(command -v iptables)"
ban_ipt4_savecmd="$(command -v iptables-save)"
ban_ipt4_restorecmd="$(command -v iptables-restore)"
ban_ipt6_cmd="$(command -v ip6tables)"
ban_ipt6_savecmd="$(command -v ip6tables-save)"
ban_ipt6_restorecmd="$(command -v ip6tables-restore)"
ban_ipset_cmd="$(command -v ipset)"
ban_logger_cmd="$(command -v logger)"
ban_logread_cmd="$(command -v logread)"
ban_allsources=""
ban_sources=""
ban_asns=""
ban_countries=""
ban_settype_src=""
ban_settype_dst=""
ban_settype_all=""
ban_lan_inputchains_4=""
ban_lan_inputchains_6=""
ban_lan_forwardchains_4=""
ban_lan_forwardchains_6=""
ban_wan_inputchains_4=""
ban_wan_inputchains_6=""
ban_wan_forwardchains_4=""
ban_wan_forwardchains_6=""
ban_action="${1:-"start"}"
ban_pidfile="/var/run/banip.pid"
ban_bgpidfile="/var/run/banip_bg.pid"
ban_tmpbase="/tmp"
ban_rtfile="${ban_tmpbase}/ban_runtime.json"
ban_srcfile="${ban_tmpbase}/ban_sources.json"
ban_reportdir="${ban_tmpbase}/banIP-Report"
ban_backupdir="${ban_tmpbase}/banIP-Backup"
ban_srcarc="/etc/banip/banip.sources.gz"
ban_dnsservice="/etc/banip/banip.dns"
ban_mailservice="/etc/banip/banip.mail"
ban_logservice="/etc/banip/banip.service"
ban_maclist="/etc/banip/banip.maclist"
ban_blacklist="/etc/banip/banip.blacklist"
ban_whitelist="/etc/banip/banip.whitelist"
ban_setcnt="0"
ban_cnt="0"

# load environment
#
f_load()
{
	# get system information
	#
	ban_sysver="$(ubus -S call system board 2>/dev/null | jsonfilter -e '@.model' -e '@.release.description' | \
		awk 'BEGIN{ORS=", "}{print $0}' | awk '{print substr($0,1,length($0)-2)}')"

	# load config
	#
	f_conf

	# check status
	#
	if [ "${ban_enabled}" = "0" ]
	then
		f_bgsrv "stop"
		f_ipset "destroy"
		f_jsnup "disabled"
		f_rmbckp
		f_rmtmp
		f_log "info" "banIP is currently disabled, please set the config option 'ban_enabled' to '1' to use this service"
		exit 0
	fi

	f_dir "${ban_backupdir}"
	f_dir "${ban_reportdir}"
}

# check/create directories
#
f_dir()
{
	local dir="${1}"

	if [ ! -d "${dir}" ]
	then
		mkdir -p "${dir}"
		if [ "${?}" = "0" ]
		then
			f_log "debug" "directory '${dir}' created"
		else
			f_log "err" "directory '${dir}' could not be created"
		fi
	else
		f_log "debug" "directory '${dir}' is used"
	fi
}

# load banIP config
#
f_conf()
{
	if [ ! -r "/etc/config/banip" ] || [ -z "$(uci -q show banip.global.ban_autodetect)" ]
	then
		f_log "err" "no valid banIP config found, please re-install the package via opkg with the '--force-reinstall --force-maintainer' options"
	fi

	config_cb()
	{
		option_cb()
		{
			local option="${1}"
			local value="${2}"
			eval "${option}=\"${value}\""
		}
		list_cb()
		{
			local option="${1}"
			local value="${2}"
			if [ "${option}" = "ban_ifaces" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_ifaces}")${value} \""
			elif [ "${option}" = "ban_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_sources}")${value} \""
			elif [ "${option}" = "ban_localsources" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_localsources}")${value} \""
			elif [ "${option}" = "ban_extrasources" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_extrasources}")${value} \""
			elif [ "${option}" = "ban_settype_src" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_settype_src}")${value} \""
			elif [ "${option}" = "ban_settype_dst" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_settype_dst}")${value} \""
			elif [ "${option}" = "ban_settype_all" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_settype_all}")${value} \""
			elif [ "${option}" = "ban_mailactions" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_mailactions}")${value} \""
			elif [ "${option}" = "ban_logterms" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_logterms}")${value} \""
			elif [ "${option}" = "ban_countries" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_countries}")${value} \""
			elif [ "${option}" = "ban_asns" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_asns}")${value} \""
			elif [ "${option}" = "ban_lan_inputchains_4" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_lan_inputchains_4}")${value} \""
			elif [ "${option}" = "ban_lan_inputchains_6" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_lan_inputchains_6}")${value} \""
			elif [ "${option}" = "ban_lan_forwardchains_4" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_lan_forwardchains_4}")${value} \""
			elif [ "${option}" = "ban_lan_forwardchains_6" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_lan_forwardchains_6}")${value} \""
			elif [ "${option}" = "ban_wan_inputchains_4" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_wan_inputchains_4}")${value} \""
			elif [ "${option}" = "ban_wan_inputchains_6" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_wan_inputchains_6}")${value} \""
			elif [ "${option}" = "ban_wan_forwardchains_4" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_wan_forwardchains_4}")${value} \""
			elif [ "${option}" = "ban_wan_forwardchains_6" ]
			then
				eval "${option}=\"$(printf "%s" "${ban_wan_forwardchains_6}")${value} \""
			fi
		}
	}
	config_load banip

	ban_chain="${ban_chain:-"banIP"}"
	ban_global_settype="${ban_global_settype:-"src+dst"}"
	ban_target_src="${ban_target_src:-"DROP"}"
	ban_target_dst="${ban_target_dst:-"REJECT"}"
	ban_lan_inputchains_4="${ban_lan_inputchains_4:-"input_lan_rule"}"
	ban_lan_inputchains_6="${ban_lan_inputchains_6:-"input_lan_rule"}"
	ban_lan_forwardchains_4="${ban_lan_forwardchains_4:-"forwarding_lan_rule"}"
	ban_lan_forwardchains_6="${ban_lan_forwardchains_6:-"forwarding_lan_rule"}"
	ban_wan_inputchains_4="${ban_wan_inputchains_4:-"input_wan_rule"}"
	ban_wan_inputchains_6="${ban_wan_inputchains_6:-"input_wan_rule"}"
	ban_wan_forwardchains_4="${ban_wan_forwardchains_4:-"forwarding_wan_rule"}"
	ban_wan_forwardchains_6="${ban_wan_forwardchains_6:-"forwarding_wan_rule"}"
	ban_logchain_src="${ban_logchain_src:-"${ban_chain}_log_src"}"
	ban_logchain_dst="${ban_logchain_dst:-"${ban_chain}_log_dst"}"
	ban_logtarget_src="${ban_target_src}"
	ban_logtarget_dst="${ban_target_dst}"
	if [ "${ban_logsrc_enabled}" = "1" ]
	then
		ban_logprefix_src="${ban_logprefix_src:-"[banIP-${ban_ver%-*}, src/${ban_target_src}] "}"
		ban_logopts_src="${ban_logopts_src:-"-m limit --limit 2/sec"}"
		ban_target_src="${ban_logchain_src}"
	fi
	if [ "${ban_logdst_enabled}" = "1" ]
	then
		ban_logprefix_dst="${ban_logprefix_dst:-"[banIP-${ban_ver%-*}, dst/${ban_target_dst}] "}"
		ban_logopts_dst="${ban_logopts_dst:-"-m limit --limit 2/sec"}"
		ban_target_dst="${ban_logchain_dst}"
	fi
	ban_localsources="${ban_localsources:-"maclist whitelist blacklist"}"
	ban_logterms="${ban_logterms:-"dropbear sshd luci nginx"}"
	f_log "debug" "f_conf  ::: ifaces: ${ban_ifaces:-"-"}, chain: ${ban_chain}, set_type: ${ban_global_settype}, log_chains (src/dst): ${ban_logchain_src}/${ban_logchain_dst}, targets (src/dst): ${ban_target_src}/${ban_target_dst}, whitelist_only: ${ban_whitelistonly}"
	f_log "debug" "f_conf  ::: lan_inputs (4/6): ${ban_lan_inputchains_4}/${ban_lan_inputchains_6}, lan_forwards (4/6): ${ban_lan_forwardchains_4}/${ban_lan_forwardchains_6}, wan_inputs (4/6): ${ban_wan_inputchains_4}/${ban_wan_inputchains_6}, wan_forwards (4/6): ${ban_wan_forwardchains_4}/${ban_wan_forwardchains_6}"
	f_log "debug" "f_conf  ::: local_sources: ${ban_localsources:-"-"}, extra_sources: ${ban_extrasources:-"-"}, log_terms: ${ban_logterms:-"-"}, log_prefixes (src/dst): ${ban_logprefix_src}/${ban_logprefix_dst}, log_options (src/dst): ${ban_logopts_src}/${ban_logopts_dst}"
}

# check environment
#
f_env()
{
	local util utils packages iface insecure tmp cnt="0" cnt_max="10"

	ban_starttime="$(date "+%s")"
	f_jsnup "running"
	f_log "info" "start banIP processing (${ban_action})"

	# create temp directory & files
	#
	f_tmp

	# get wan devices and wan subnets
	#
	if [ "${ban_autodetect}" = "1" ] && [ -z "${ban_ifaces}" ]
	then
		while [ "${cnt}" -le "${cnt_max}" ]
		do
			network_find_wan iface
			if [ -n "${iface}" ] && [ -z "$(printf "%s\n" "${ban_ifaces}" | grep -F "${iface}")" ]
			then
				ban_proto4_enabled="1"
				ban_ifaces="${ban_ifaces}${iface} "
				uci_set banip global ban_proto4_enabled "1"
				uci_add_list banip global ban_ifaces "${iface}"
			fi
			network_find_wan6 iface
			if [ -n "${iface}" ] && [ -z "$(printf "%s\n" "${ban_ifaces}" | grep -F "${iface}")" ]
			then
				ban_proto6_enabled="1"
				ban_ifaces="${ban_ifaces}${iface} "
				uci_set banip global ban_proto6_enabled "1"
				uci_add_list banip global ban_ifaces "${iface}"
			fi
			if [ -z "${ban_ifaces}" ]
			then
				if [ "${cnt}" -le "${cnt_max}" ]
				then
					network_flush_cache
					cnt=$((cnt+1))
					sleep 1
				else
					break
				fi
			else
				if [ -n "$(uci -q changes "banip")" ]
				then
					uci_commit "banip"
				fi
				break
			fi
		done
	fi

	while [ "${cnt}" -le "${cnt_max}" ]
	do
		for iface in ${ban_ifaces}
		do
			network_get_device tmp "${iface}"
			if [ -n "${tmp}" ] && [ -z "$(printf "%s\n" "${ban_devs}" | grep -F "${tmp}")" ]
			then
				ban_devs="${ban_devs} ${tmp}"
			else
				network_get_physdev tmp "${iface}"
				if [ -n "${tmp}" ] && [ -z "$(printf "%s\n" "${ban_devs}" | grep -F "${tmp}")" ]
				then
					ban_devs="${ban_devs} ${tmp}"
				fi
			fi
			network_get_subnet tmp "${iface}"
			if [ -n "${tmp}" ] && [ -z "$(printf "%s\n" "${ban_subnets}" | grep -F "${tmp}")" ]
			then
				ban_subnets="${ban_subnets} ${tmp}"
			fi
			network_get_subnet6 tmp "${iface}"
			if [ -n "${tmp}" ] && [ -z "$(printf "%s\n" "${ban_subnets}" | grep -F "${tmp}")" ]
			then
				ban_subnets="${ban_subnets} ${tmp}"
			fi
		done
		if [ -z "${ban_devs}" ] || [ -z "${ban_subnets}" ]
		then
			if [ "${cnt}" -le "${cnt_max}" ]
			then
				network_flush_cache
				cnt=$((cnt+1))
				sleep 1
			else
				break
			fi
		else
			break
		fi
	done
	ban_ipdevs="$("${ban_ip_cmd}" link show 2>/dev/null | awk 'BEGIN{FS="[@: ]"}/^[0-9:]/{if($3!="lo"){ORS=" ";print $3}}')"

	if [ -z "${ban_ifaces}" ] || [ -z "${ban_devs}" ] || [ -z "${ban_ipdevs}" ]
	then
		f_log "err" "logical wan interface(s)/device(s) '${ban_ifaces:-"-"}/${ban_devs:-"-"}' not found, please please check your configuration"
	elif [ -z "${ban_ipdevs}" ]
	then
		f_log "err" "ip device(s) '${ban_ipdevs:-"-"}' not found, please please check your configuration"
	fi

	# check ipset/iptables utility
	#
	if [ ! -x "${ban_ipset_cmd}" ]
	then
		f_log "err" "ipset utility '${ban_ipset_cmd:-"-"}' not executable, please install package 'ipset'"
	fi
	if { [ "${ban_proto4_enabled}" = "1" ] && { [ ! -x "${ban_ipt4_cmd}" ] || [ ! -x "${ban_ipt4_savecmd}" ] || [ ! -x "${ban_ipt4_restorecmd}" ]; }; } || \
		{ [ "${ban_proto6_enabled}" = "1" ] && { [ ! -x "${ban_ipt6_cmd}" ] || [ ! -x "${ban_ipt6_savecmd}" ] || [ ! -x "${ban_ipt6_restorecmd}" ]; }; }
	then
		f_log "err" "iptables utilities '${ban_ipt4_cmd:-"-"}, ${ban_ipt4_savecmd:-"-"}, ${ban_ipt4_restorecmd:-"-"}/${ban_ipt6_cmd:-"-"}', ${ban_ipt6_savecmd:-"-"}, ${ban_ipt6_restorecmd:-"-"} not executable, please install the relevant iptables packages"
	fi

	# check download utility
	#
	if [ -z "${ban_fetchutil}" ]
	then
		while [ -z "${packages}" ] && [ "${cnt}" -le "${cnt_max}" ]
		do
			packages="$(opkg list-installed 2>/dev/null)"
			cnt=$((cnt+1))
			sleep 1
		done
		if [ -z "${packages}" ]
		then
			f_log "err" "local opkg package repository is not available, please set 'ban_fetchutil' manually"
		fi

		utils="aria2c curl wget uclient-fetch"
		for util in ${utils}
		do
			if { [ "${util}" = "uclient-fetch" ] && [ -n "$(printf "%s" "${packages}" | grep "^libustream-")" ]; } || \
				{ [ "${util}" = "wget" ] && [ -n "$(printf "%s" "${packages}" | grep "^wget -")" ]; } || \
				[ "${util}" = "curl" ] || [ "${util}" = "aria2c" ]
			then
				if [ -x "$(command -v "${util}")" ]
				then
					ban_fetchutil="${util}"
					uci_set banip global ban_fetchutil "${util}"
					uci_commit "banip"
					break
				fi
			fi
		done
	elif [ ! -x "$(command -v "${ban_fetchutil}")" ]
	then
		unset ban_fetchutil
	fi
	case "${ban_fetchutil}" in
		"aria2c")
			if [ "${ban_fetchinsecure}" = "1" ]
			then
				insecure="--check-certificate=false"
			fi
			ban_fetchparm="${ban_fetchparm:-"${insecure} --timeout=20 --allow-overwrite=true --auto-file-renaming=false --log-level=warn --dir=/ -o"}"
		;;
		"curl")
			if [ "${ban_fetchinsecure}" = "1" ]
			then
				insecure="--insecure"
			fi
			ban_fetchparm="${ban_fetchparm:-"${insecure} --connect-timeout 20 --silent --show-error --location -o"}"
		;;
		"uclient-fetch")
			if [ "${ban_fetchinsecure}" = "1" ]
			then
				insecure="--no-check-certificate"
			fi
			ban_fetchparm="${ban_fetchparm:-"${insecure} --timeout=20 -O"}"
		;;
		"wget")
			if [ "${ban_fetchinsecure}" = "1" ]
			then
				insecure="--no-check-certificate"
			fi
			ban_fetchparm="${ban_fetchparm:-"${insecure} --no-cache --no-cookies --max-redirect=0 --timeout=20 -O"}"
		;;
	esac
	if [ -n "${ban_fetchutil}" ] && [ -n "${ban_fetchparm}" ]
	then
		ban_fetchutil="$(command -v "${ban_fetchutil}")"
	else
		f_log "err" "download utility with SSL support not found, please install 'uclient-fetch' with a 'libustream-*' variant or another download utility like 'wget', 'curl' or 'aria2'"
	fi

	# load JSON source file
	#
	if [ ! -r "${ban_srcfile}" ]
	then
		if [ -r "${ban_srcarc}" ]
		then
			zcat "${ban_srcarc}" > "${ban_srcfile}"
		else
			f_log "err" "banIP source archive not found"
		fi
	fi
	if [ -r "${ban_srcfile}" ]
	then
		json_load_file "${ban_srcfile}"
		json_get_keys ban_allsources
		ban_allsources="${ban_allsources} maclist blacklist whitelist"
	else
		f_log "err" "banIP source file not found"
	fi
	f_log "debug" "f_env   ::: auto_detect: ${ban_autodetect}, fetch_util: ${ban_fetchutil:-"-"}, fetch_parm: ${ban_fetchparm:-"-"}, src_file: ${ban_srcfile:-"-"}, log_terms: ${ban_logterms}, interfaces: ${ban_ifaces:-"-"}, devices: ${ban_devs:-"-"}, subnets: ${ban_subnets:-"-"}, ip_devices: ${ban_ipdevs:-"-"}, protocols (4/6): ${ban_proto4_enabled}/${ban_proto6_enabled}"
}

# create temporary files and directories
#
f_tmp()
{
	f_dir "${ban_tmpbase}"

	ban_tmpdir="$(mktemp -p "${ban_tmpbase}" -d)"
	ban_tmpfile="$(mktemp -p "${ban_tmpdir}" -tu)"

	if [ ! -f "${ban_pidfile}" ] || [ ! -s "${ban_pidfile}" ]
	then
		printf "%s" "${$}" > "${ban_pidfile}"
	fi
	f_log "debug" "f_tmp   ::: tmp_base: ${ban_tmpbase:-"-"}, tmp_dir: ${ban_tmpdir:-"-"}, pid_file: ${ban_pidfile:-"-"}"
}

# remove temporary files and directories
#
f_rmtmp()
{
	if [ -d "${ban_tmpdir}" ]
	then
		rm -rf "${ban_tmpdir}"
	fi
	rm -f "${ban_srcfile}"
	> "${ban_pidfile}"
	f_log "debug" "f_rmtmp ::: tmp_base: ${ban_tmpbase:-"-"}, tmp_dir: ${ban_tmpdir:-"-"}, pid_file: ${ban_pidfile:-"-"}"
}

# remove backup files
#
f_rmbckp()
{
	if [ -d "${ban_backupdir}" ]
	then
		rm -f "${ban_backupdir}/banIP."*".gz"
	fi
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

# apply iptables rules
#
f_iptrule()
{
	local rc timeout="-w 5" action="${1}" chain="${2}" rule="${3}" pos="${4}"

	if [ "${ban_proto4_enabled}" = "1" ] && { [ "${src_name}" = "maclist" ] || [ "${src_name##*_}" = "4" ]; }
	then
		rc="$("${ban_ipt4_cmd}" "${timeout}" -C ${chain} ${rule} 2>/dev/null; printf "%u" ${?})"
		if { [ "${rc}" != "0" ] && { [ "${action}" = "-A" ] || [ "${action}" = "-I" ]; }; } || \
			{ [ "${rc}" = "0" ] && [ "${action}" = "-D" ]; }
		then
			"${ban_ipt4_cmd}" "${timeout}" "${action}" ${chain} ${pos} ${rule} 2>/dev/null
			rc="${?}"
		else
			rc=0
		fi
	fi
	if [ "${ban_proto6_enabled}" = "1" ] && { [ "${src_name}" = "maclist" ] || [ "${src_name##*_}" = "6" ]; }
	then
		rc="$("${ban_ipt6_cmd}" "${timeout}" -C ${chain} ${rule} 2>/dev/null; printf "%u" ${?})"
		if { [ "${rc}" != "0" ] && { [ "${action}" = "-A" ] || [ "${action}" = "-I" ]; }; } || \
			{ [ "${rc}" = "0" ] && [ "${action}" = "-D" ]; }
		then
			"${ban_ipt6_cmd}" "${timeout}" "${action}" ${chain} ${pos} ${rule} 2>/dev/null
			rc="${?}"
		else
			rc=0
		fi
	fi
	if [ -n "${rc}" ] && [ "${rc}" != "0" ]
	then
		> "${tmp_err}"
		f_log "info" "${src_name}: iptables action '${action:-"-"}' failed with '${chain}, ${pos:-"-"}, ${rule:-"-"}'"
	fi
}

# iptables controller
#
f_iptables()
{
	local ipt_cmd chain chainsets dev pos timeout="-w 5" destroy="${1}"

	if [ "${ban_action}" != "refresh" ] && [ "${ban_action}" != "resume" ]
	then
		for dev in ${ban_ipdevs}
		do
			if [ "${src_name}" = "maclist" ]
			then
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} src -j RETURN"
			elif [ "${src_name%_*}" = "whitelist" ]
			then
				f_iptrule "-D" "${ban_chain}" "-i ${dev} -m set ! --match-set ${src_name} src -j ${ban_logtarget_src}"
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set ! --match-set ${src_name} dst -j ${ban_logtarget_dst}"
				f_iptrule "-D" "${ban_chain}" "-i ${dev} -m set ! --match-set ${src_name} src -j ${ban_logchain_src}"
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set ! --match-set ${src_name} dst -j ${ban_logchain_dst}"
				f_iptrule "-D" "${ban_chain}" "-i ${dev} -m set --match-set ${src_name} src -j RETURN"
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} dst -j RETURN"
			else
				f_iptrule "-D" "${ban_chain}" "-i ${dev} -m set --match-set ${src_name} src -j ${ban_logtarget_src}"
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} dst -j ${ban_logtarget_dst}"
				f_iptrule "-D" "${ban_chain}" "-i ${dev} -m set --match-set ${src_name} src -j ${ban_logchain_src}"
				f_iptrule "-D" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} dst -j ${ban_logchain_dst}"
			fi
		done
	fi
	if [ -z "${destroy}" ] && { [ "${cnt}" -gt "0" ] || [ "${src_name%_*}" = "blacklist" ] || [ "${src_name%_*}" = "whitelist" ]; }
	then
		if [ "${src_name##*_}" = "4" ]
		then
			ipt_cmd="${ban_ipt4_cmd}"
			if [ ! -f "${ban_tmpfile}.${src_name##*_}.chains" ]
			then
				> "${ban_tmpfile}.${src_name##*_}.chains"
				chainsets="${ban_lan_inputchains_4} ${ban_wan_inputchains_4} ${ban_lan_forwardchains_4} ${ban_wan_forwardchains_4}"
				for chain in ${chainsets}
				do
					f_iptrule "-I" "${chain}" "-j ${ban_chain}"
				done
				f_iptrule "-A" "${ban_chain}" "-p udp --dport 67:68 --sport 67:68 -j RETURN"
				f_iptrule "-A" "${ban_chain}" "-m conntrack ! --ctstate NEW -j RETURN"
			fi
		elif [ "${src_name##*_}" = "6" ]
		then
			ipt_cmd="${ban_ipt6_cmd}"
			if [ ! -f "${ban_tmpfile}.${src_name##*_}.chains" ]
			then
				> "${ban_tmpfile}.${src_name##*_}.chains"
				chainsets="${ban_lan_inputchains_6} ${ban_wan_inputchains_6} ${ban_lan_forwardchains_6} ${ban_wan_forwardchains_6}"
				for chain in ${chainsets}
				do
					f_iptrule "-I" "${chain}" "-j ${ban_chain}"
				done
				f_iptrule "-A" "${ban_chain}" "-p ipv6-icmp -s fe80::/10 -d fe80::/10 -j RETURN"
				f_iptrule "-A" "${ban_chain}" "-p udp -s fc00::/6 --sport 547 -d fc00::/6 --dport 546 -j RETURN"
				f_iptrule "-A" "${ban_chain}" "-m conntrack ! --ctstate NEW -j RETURN"
			fi
		fi
		if [ "${src_settype}" != "dst" ]
		then
			for dev in ${ban_devs}
			do
				if [ "${src_name}" = "maclist" ]
				then
					f_iptrule "-I" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} src -j RETURN" "1"
				elif [ "${src_name%_*}" = "whitelist" ]
				then
					pos="$(( $("${ipt_cmd}" "${timeout}" -vnL "${ban_chain}" --line-numbers | grep -cF "RETURN")+1))"
					if [ "${ban_whitelistonly}" = "1" ]
					then
						f_iptrule "-I" "${ban_chain}" "-i ${dev} -m set ! --match-set ${src_name} src -j ${ban_target_src}" "${pos}"
					else
						f_iptrule "-I" "${ban_chain}" "-i ${dev} -m set --match-set ${src_name} src -j RETURN" "${pos}"
					fi
				else
					f_iptrule "${action:-"-A"}" "${ban_chain}" "-i ${dev} -m set --match-set ${src_name} src -j ${ban_target_src}"
				fi
			done
		fi
		if [ "${src_settype}" != "src" ]
		then
			for dev in ${ban_devs}
			do
				if [ "${src_name%_*}" = "whitelist" ]
				then
					pos="$(( $("${ipt_cmd}" "${timeout}" -vnL "${ban_chain}" --line-numbers | grep -cF "RETURN")+1))"
					if [ "${ban_whitelistonly}" = "1" ]
					then
						f_iptrule "-I" "${ban_chain}" "-o ${dev} -m set ! --match-set ${src_name} dst -j ${ban_target_dst}" "${pos}"
					else
						f_iptrule "-I" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} dst -j RETURN" "${pos}"
					fi
				elif [ "${src_name}" != "maclist" ]
				then
					f_iptrule "${action:-"-A"}" "${ban_chain}" "-o ${dev} -m set --match-set ${src_name} dst -j ${ban_target_dst}"
				fi
			done
		fi
	else
		"${ban_ipset_cmd}" -q destroy "${src_name}"
	fi
}

# ipset controller
#
f_ipset()
{
	local src src_list action rule ipt_cmd out_rc max="0" cnt="0" cnt_ip="0" cnt_cidr="0" cnt_mac="0" timeout="-w 5" mode="${1}" in_rc="4"

	case "${mode}" in
		"backup")
			gzip -cf "${tmp_load}" 2>/dev/null > "${ban_backupdir}/banIP.${src_name}.gz"
			out_rc="${?}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"restore")
			if [ -f "${ban_backupdir}/banIP.${src_name}.gz" ]
			then
				zcat "${ban_backupdir}/banIP.${src_name}.gz" 2>/dev/null > "${tmp_load}"
				out_rc="${?}"
			else
				out_rc="${in_rc}"
			fi
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"remove")
			if [ -f "${ban_backupdir}/banIP.${src_name}.gz" ]
			then
				rm -f "${ban_backupdir}/banIP.${src_name}.gz"
				out_rc="${?}"
			else
				out_rc="${in_rc}"
			fi
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"initial")
			for proto in "4" "6"
			do
				if [ "${proto}" = "4" ] && [ "${ban_proto4_enabled}" = "1" ]
				then
					ipt_cmd="${ban_ipt4_cmd}"
					chainsets="${ban_lan_inputchains_4} ${ban_lan_forwardchains_4} ${ban_wan_inputchains_4} ${ban_wan_forwardchains_4}"
				elif [ "${proto}" = "6" ] && [ "${ban_proto6_enabled}" = "1" ]
				then
					ipt_cmd="${ban_ipt6_cmd}"
					chainsets="${ban_lan_inputchains_6} ${ban_lan_forwardchains_6} ${ban_wan_inputchains_6} ${ban_wan_forwardchains_6}"
				fi

				if { [ "${proto}" = "4" ] && [ "${ban_proto4_enabled}" = "1" ]; } || \
					{ [ "${proto}" = "6" ] && [ "${ban_proto6_enabled}" = "1" ]; }
				then
					if [ -z "$("${ipt_cmd}" "${timeout}" -nL "${ban_chain}" 2>/dev/null)" ]
					then
						"${ipt_cmd}" "${timeout}" -N "${ban_chain}" 2>/dev/null
						out_rc="${?}"
						f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, chain: ${ban_chain:-"-"}, out_rc: ${out_rc}"
					else
						out_rc=0
						for chain in ${chainsets}
						do
							f_iptrule "-D" "${chain}" "-j ${ban_chain}"
						done
					fi

					if [ "${ban_logsrc_enabled}" = "1" ] && [ "${out_rc}" = "0" ] && [ -z "$("${ipt_cmd}" "${timeout}" -nL "${ban_logchain_src}" 2>/dev/null)" ]
					then
						"${ipt_cmd}" "${timeout}" -N "${ban_logchain_src}" 2>/dev/null
						out_rc="${?}"
						if [ "${out_rc}" = "0" ]
						then
							"${ipt_cmd}" "${timeout}" -A "${ban_logchain_src}" -j LOG ${ban_logopts_src} --log-prefix "${ban_logprefix_src}"
							out_rc="${?}"
							if [ "${out_rc}" = "0" ]
							then
								"${ipt_cmd}" "${timeout}" -A "${ban_logchain_src}" -j "${ban_logtarget_src}"
								out_rc="${?}"
							fi
						fi
						f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, logchain_src: ${ban_logchain_src:-"-"}, out_rc: ${out_rc}"
					fi

					if [ "${ban_logdst_enabled}" = "1" ] && [ "${out_rc}" = "0" ] && [ -z "$("${ipt_cmd}" "${timeout}" -nL "${ban_logchain_dst}" 2>/dev/null)" ]
					then
						"${ipt_cmd}" "${timeout}" -N "${ban_logchain_dst}" 2>/dev/null
						out_rc="${?}"
						if [ "${out_rc}" = "0" ]
						then
							"${ipt_cmd}" "${timeout}" -A "${ban_logchain_dst}" -j LOG ${ban_logopts_dst} --log-prefix "${ban_logprefix_dst}"
							out_rc="${?}"
							if [ "${out_rc}" = "0" ]
							then
								"${ipt_cmd}" "${timeout}" -A "${ban_logchain_dst}" -j "${ban_logtarget_dst}"
								out_rc="${?}"
							fi
						fi
						f_log "debug" "f_ipset ::: name: initial, mode: ${mode:-"-"}, logchain_dst: ${ban_logchain_dst:-"-"}, out_rc: ${out_rc}"
					fi
				fi
			done
			out_rc="${out_rc:-"${in_rc}"}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"create")
			if [ -z "$("${ban_ipset_cmd}" -q -n list "${src_name}")" ] && \
				{ [ -s "${tmp_file}" ] || [ "${src_name%_*}" = "whitelist" ] || [ "${src_name%_*}" = "blacklist" ]; }
			then
				max="$(awk 'END{print NR}' "${tmp_file}" 2>/dev/null)"
				max=$((max+262144))
				if [ "${src_name}" = "maclist" ]
				then
					"${ban_ipset_cmd}" create "${src_name}" hash:mac hashsize 64 maxelem "${max}" counters timeout "${ban_maclist_timeout:-"0"}"
					out_rc="${?}"
				elif [ "${src_name%_*}" = "whitelist" ]
				then
					"${ban_ipset_cmd}" create "${src_name}" hash:net hashsize 64 maxelem "${max}" family "${src_ipver}" counters timeout "${ban_whitelist_timeout:-"0"}"
					out_rc="${?}"
				elif [ "${src_name%_*}" = "blacklist" ]
				then
					"${ban_ipset_cmd}" create "${src_name}" hash:net hashsize 64 maxelem "${max}" family "${src_ipver}" counters timeout "${ban_blacklist_timeout:-"0"}"
					out_rc="${?}"
				else
					"${ban_ipset_cmd}" create "${src_name}" hash:net hashsize 64 maxelem "${max}" family "${src_ipver}" counters
					out_rc="${?}"
				fi
			elif [ -n "$("${ban_ipset_cmd}" -q -n list "${src_name}")" ]
			then
				"${ban_ipset_cmd}" -q flush "${src_name}"
				out_rc="${?}"
			fi
			if [ -s "${tmp_file}" ] && [ "${out_rc}" = "0" ]
			then
				"${ban_ipset_cmd}" -q -! restore < "${tmp_file}"
				out_rc="${?}"
				if [ "${out_rc}" = "0" ]
				then
					src_list="$("${ban_ipset_cmd}" -q list "${src_name}")"
					cnt="$(printf "%s\n" "${src_list}" | awk '/^Number of entries:/{print $4}')"
					cnt_mac="$(printf "%s\n" "${src_list}" | grep -cE "^(([0-9A-Z][0-9A-Z]:){5}[0-9A-Z]{2} )")"
					cnt_cidr="$(printf "%s\n" "${src_list}" | grep -cE "(/[0-9]{1,3} )")"
					cnt_ip=$((cnt-cnt_cidr-cnt_mac))
					printf "%s\n" "${cnt}" > "${tmp_cnt}"
				fi
			fi
			f_iptables
			end_ts="$(date +%s)"
			out_rc="${out_rc:-"${in_rc}"}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, ipver: ${src_ipver:-"-"}, settype: ${src_settype:-"-"}, count(sum/ip/cidr/mac): ${cnt}/${cnt_ip}/${cnt_cidr}/${cnt_mac}, time: $((end_ts-start_ts)), out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"refresh")
			if [ -n "$("${ban_ipset_cmd}" -q -n list "${src_name}")" ]
			then
				out_rc=0
				src_list="$("${ban_ipset_cmd}" -q list "${src_name}")"
				cnt="$(printf "%s\n" "${src_list}" | awk '/^Number of entries:/{print $4}')"
				cnt_mac="$(printf "%s\n" "${src_list}" | grep -cE "^(([0-9A-Z][0-9A-Z]:){5}[0-9A-Z]{2} )")"
				cnt_cidr="$(printf "%s\n" "${src_list}" | grep -cE "(/[0-9]{1,3} )")"
				cnt_ip=$((cnt-cnt_cidr-cnt_mac))
				printf "%s\n" "${cnt}" > "${tmp_cnt}"
				f_iptables
			fi
			end_ts="$(date +%s)"
			out_rc="${out_rc:-"${in_rc}"}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, count(sum/ip/cidr/mac): ${cnt}/${cnt_ip}/${cnt_cidr}/${cnt_mac}, time: $((end_ts-start_ts)), out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"suspend")
			for src in ${ban_sources} ${ban_localsources}
			do
				if [ "${src}" = "maclist" ] && [ -n "$("${ban_ipset_cmd}" -q -n list "${src}")" ]
				then
					tmp_file="${ban_backupdir}/${src}.file"
					"${ban_ipset_cmd}" -q save "${src}" | tail -n +2 > "${tmp_file}"
					"${ban_ipset_cmd}" -q flush "${src}"
				else
					for proto in "4" "6"
					do
						if [ -n "$("${ban_ipset_cmd}" -q -n list "${src}_${proto}")" ]
						then
							tmp_file="${ban_backupdir}/${src}_${proto}.file"
							"${ban_ipset_cmd}" -q save "${src}_${proto}" | tail -n +2 > "${tmp_file}"
							"${ban_ipset_cmd}" -q flush "${src}_${proto}"
						fi
					done
				fi
			done
			f_log "debug" "f_ipset ::: name: ${src:-"-"}, mode: ${mode:-"-"}"
		;;
		"resume")
			if [ -f "${ban_backupdir}/${src_name}.file" ]
			then
				"${ban_ipset_cmd}" -q -! restore < "${ban_backupdir}/${src_name}.file"
				out_rc="${?}"
				if [ "${out_rc}" = "0" ]
				then
					rm -f "${ban_backupdir}/${src_name}.file"
					src_list="$("${ban_ipset_cmd}" -q list "${src_name}")"
					cnt="$(printf "%s\n" "${src_list}" | awk '/^Number of entries:/{print $4}')"
					cnt_mac="$(printf "%s\n" "${src_list}" | grep -cE "^(([0-9A-Z][0-9A-Z]:){5}[0-9A-Z]{2} )")"
					cnt_cidr="$(printf "%s\n" "${src_list}" | grep -cE "(/[0-9]{1,3} )")"
					cnt_ip=$((cnt-cnt_cidr-cnt_mac))
					printf "%s\n" "${cnt}" > "${tmp_cnt}"
				fi
				f_iptables
			fi
			end_ts="$(date +%s)"
			out_rc="${out_rc:-"${in_rc}"}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, ipver: ${src_ipver:-"-"}, settype: ${src_settype:-"-"}, count(sum/ip/cidr/mac): ${cnt}/${cnt_ip}/${cnt_cidr}/${cnt_mac}, time: $((end_ts-start_ts)), out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"flush")
			if [ -n "$("${ban_ipset_cmd}" -q -n list "${src_name}")" ]
			then
				f_iptables "destroy"
				out_rc=0
			fi
			out_rc="${out_rc:-"${in_rc}"}"
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}, out_rc: ${out_rc}"
			return "${out_rc}"
		;;
		"destroy")
			for chain in ${ban_chain} ${ban_logchain_src} ${ban_logchain_dst}
			do
				if [ -n "$("${ban_ipt4_cmd}" "${timeout}" -nL "${chain}" 2>/dev/null)" ]
				then
					"${ban_ipt4_savecmd}" | grep -v -- "-j ${chain}" | "${ban_ipt4_restorecmd}"
					"${ban_ipt4_cmd}" "${timeout}" -F "${chain}" 2>/dev/null
					"${ban_ipt4_cmd}" "${timeout}" -X "${chain}" 2>/dev/null
				fi
				if [ -n "$("${ban_ipt6_cmd}" "${timeout}" -nL "${chain}" 2>/dev/null)" ]
				then
					"${ban_ipt6_savecmd}" | grep -v -- "-j ${chain}" | "${ban_ipt6_restorecmd}"
					"${ban_ipt6_cmd}" "${timeout}" -F "${chain}" 2>/dev/null
					"${ban_ipt6_cmd}" "${timeout}" -X "${chain}" 2>/dev/null
				fi
			done
			for src in ${ban_sources} maclist blacklist whitelist
			do
				if [ "${src}" = "maclist" ] && [ -n "$("${ban_ipset_cmd}" -q -n list "${src}")" ]
				then
					"${ban_ipset_cmd}" -q destroy "${src}"
				else
					for proto in "4" "6"
					do
						if [ -n "$("${ban_ipset_cmd}" -q -n list "${src}_${proto}")" ]
						then
							"${ban_ipset_cmd}" -q destroy "${src}_${proto}"
						fi
					done
				fi
			done
			f_log "debug" "f_ipset ::: name: ${src_name:-"-"}, mode: ${mode:-"-"}"
		;;
	esac
}

# write to syslog
#
f_log()
{
	local class="${1}" log_msg="${2}"

	if [ -n "${log_msg}" ] && { [ "${class}" != "debug" ] || [ "${ban_debug}" = "1" ]; }
	then
		if [ -x "${ban_logger_cmd}" ]
		then
			"${ban_logger_cmd}" -p "${class}" -t "banIP-${ban_ver%-*}[${$}]" "${log_msg}"
		else
			printf "%s %s %s\n" "${class}" "banIP-${ban_ver%-*}[${$}]" "${log_msg}"
		fi
		if [ "${class}" = "err" ]
		then
			f_jsnup "error"
			f_ipset "destroy"
			f_rmbckp
			f_rmtmp
			exit 1
		fi
	fi
}

# kill all relevant background processes
#
f_pidx()
{
	local pids ppid="${1}"

	pids="$(pgrep -P "${ppid}" 2>/dev/null | awk '{ORS=" ";print $0}')"
	kill -HUP "${ppid}" "${pids}" 2>/dev/null
	> "${ban_bgpidfile}"
}

# start log service to trace failed ssh/luci logins
#
f_bgsrv()
{
	local bg_pid action="${1}"

	bg_pid="$(cat "${ban_bgpidfile}" 2>/dev/null)"
	if [ "${action}" = "start" ] && [ -x "${ban_logservice}" ] && [ "${ban_monitor_enabled}" = "1" ] && [ "${ban_whitelistonly}" = "0" ]
	then
		if [ -n "${bg_pid}" ]
		then
			f_pidx "${bg_pid}"
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "dropbear")" ]
		then
			ban_search="Exit before auth from|"
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "sshd")" ]
		then
			ban_search="${ban_search}error: maximum authentication attempts exceeded|sshd.*Connection closed by.*\[preauth\]|"
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "luci")" ]
		then
			ban_search="${ban_search}luci: failed login|"
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "nginx")" ]
		then
			ban_search="${ban_search}nginx\[[0-9]+\]:.*\[error\].*open().*client: [[:alnum:].:]+|"
		fi
		(
			"${ban_logservice}" "${ban_ver}" "${ban_search%?}" &
			printf "%s" "${!}" > "${ban_bgpidfile}"
		)
	elif { [ "${action}" = "stop" ] || [ "${ban_monitor_enabled}" = "0" ]; } && [ -n "${bg_pid}" ]
	then
		f_pidx "${bg_pid}"
	fi
	f_log "debug" "f_bgsrv ::: action: ${action:-"-"}, bg_pid (old/new): ${bg_pid}/$(cat "${ban_bgpidfile}" 2>/dev/null), monitor_enabled: ${ban_monitor_enabled:-"-"}, log_service: ${ban_logservice:-"-"}"
}

# download controller
#
f_down()
{
	local src_name="${1}" proto="${2}" src_ipver="${3}" src_url="${4}" src_rule="${5}" src_comp="${6}"
	local ip start_ts end_ts src_settype src_log src_rc tmp_load tmp_file tmp_raw tmp_cnt tmp_err

	start_ts="$(date +%s)"
	if [ -n "$(printf "%s\n" "${ban_settype_src}" | grep -F "${src_name}")" ]
	then
		src_settype="src"
	elif [ -n "$(printf "%s\n" "${ban_settype_dst}" | grep -F "${src_name}")" ]
	then
		src_settype="dst"
	elif [ -n "$(printf "%s\n" "${ban_settype_all}" | grep -F "${src_name}")" ]
	then
		src_settype="src+dst"
	else
		src_settype="${ban_global_settype}"
	fi
	src_name="${src_name}_${proto}"
	tmp_load="${ban_tmpfile}.${src_name}.load"
	tmp_file="${ban_tmpfile}.${src_name}.file"
	tmp_raw="${tmp_file}.raw"
	tmp_cnt="${tmp_file}.cnt"
	tmp_err="${tmp_file}.err"

	# 'resume' mode
	#
	if [ "${ban_action}" = "resume" ]
	then
		if [ "${src_name%_*}" = "maclist" ]
		then
			src_name="maclist"
		fi
		f_ipset "resume"
		src_rc="${?}"
		if [ "${src_rc}" = "0" ]
		then
			return
		fi
	fi

	# handle local downloads
	#
	case "${src_name%_*}" in
		"blacklist"|"whitelist")
			printf "%s\n" "0" > "${tmp_cnt}"
			awk "${src_rule}" "${src_url}" > "${tmp_file}"
			src_rc="${?}"
			if [ "${src_rc}" = "0" ]
			then
				f_ipset "create"
				if [ ! -f "${tmp_dns}" ] && { { [ "${proto}" = "4" ] && [ "${ban_proto4_enabled}" = "1" ]; } || \
					{ [ "${proto}" = "6" ] && [ "${ban_proto6_enabled}" = "1" ] && [ "${ban_proto4_enabled}" = "0" ]; }; }
				then
					tmp_dns="${ban_tmpbase}/${src_name%_*}.dns"
					src_rule="/^([[:alnum:]_-]{1,63}\\.)+[[:alpha:]]+([[:space:]]|$)/{print tolower(\$1)}"
					awk "${src_rule}" "${src_url}" > "${tmp_dns}"
					src_rc="${?}"
					if [ "${src_rc}" = "0" ] && [ -s "${tmp_dns}" ]
					then
						( "${ban_dnsservice}" "${ban_ver}" "${ban_action}" "${src_name%_*}" "${tmp_dns}" & )
					else
						rm -f "${tmp_dns}"
					fi
				fi
			else
				f_log "debug" "f_down  ::: name: ${src_name}, url: ${src_url}, rule: ${src_rule}, rc: ${src_rc}"
			fi
			return
		;;
		"maclist")
			src_name="${src_name%_*}"
			tmp_file="${ban_tmpfile}.${src_name}.file"
			tmp_cnt="${tmp_file}.cnt"
			tmp_err="${tmp_file}.err"
			awk "${src_rule}" "${src_url}" > "${tmp_file}"
			src_rc="${?}"
			if [ "${src_rc}" = "0" ]
			then
				f_ipset "create"
			else
				f_log "debug" "f_down  ::: name: ${src_name}, url: ${src_url}, rule: ${src_rule}, rc: ${src_rc}"
			fi
			return
		;;
	esac

	# 'refresh' mode
	#
	if [ "${ban_action}" = "refresh" ]
	then
		f_ipset "refresh"
		src_rc="${?}"
		if [ "${src_rc}" = "0" ]
		then
			return
		fi
	fi

	# 'start' mode
	#
	if [ "${ban_action}" = "start" ]
	then
		f_ipset "restore"
	fi
	src_rc="${?}"
	if [ "${src_rc}" = "0" ]
	then
		awk "${src_rule}" "${tmp_load}" 2>/dev/null > "${tmp_file}"
		src_rc="${?}"
		if [ "${src_rc}" = "0" ]
		then
			f_ipset "create"
			src_rc="${?}"
			if [ "${src_rc}" = "0" ]
			then
				return
			fi
		fi
	fi

	# handle country related downloads
	#
	if [ "${src_name%_*}" = "country" ]
	then
		for country in ${ban_countries}
		do
			src_log="$("${ban_fetchutil}" ${ban_fetchparm} "${tmp_raw}" "${src_url}${country}-aggregated.zone" 2>&1)"
			src_rc="${?}"
			if [ "${src_rc}" = "0" ]
			then
				cat "${tmp_raw}" 2>/dev/null >> "${tmp_load}"
			else
				continue
			fi
		done

	# handle asn related downloads
	#
	elif [ "${src_name%_*}" = "asn" ]
	then
		for asn in ${ban_asns}
		do
			src_log="$("${ban_fetchutil}" ${ban_fetchparm} "${tmp_raw}" "${src_url}AS${asn}" 2>&1)"
			src_rc="${?}"
			if [ "${src_rc}" = "0" ]
			then
				cat "${tmp_raw}" 2>/dev/null >> "${tmp_load}"
			else
				continue
			fi
		done

	# handle compressed downloads
	#
	elif [ -n "${src_comp}" ]
	then
		case "${src_comp}" in
			"gz")
				src_log="$("${ban_fetchutil}" ${ban_fetchparm} "${tmp_raw}" "${src_url}" 2>&1)"
				src_rc="${?}"
				if [ "${src_rc}" -eq 0 ]
				then
					zcat "${tmp_raw}" 2>/dev/null > "${tmp_load}"
					src_rc="${?}"
				fi
			;;
		esac

	# handle normal downloads
	#
	else
		src_log="$("${ban_fetchutil}" ${ban_fetchparm} "${tmp_load}" "${src_url}" 2>&1)"
		src_rc="${?}"
	fi

	# download post-processing (backup, restore, regex)
	#
	if [ "${src_rc}" = "0" ]
	then
		f_ipset "backup"
		src_rc="${?}"
	elif [ "${ban_action}" != "start" ] && [ "${ban_action}" != "refresh" ]
	then
		f_ipset "restore"
		src_rc="${?}"
	fi
	if [ "${src_rc}" = "0" ]
	then
		awk "${src_rule}" "${tmp_load}" 2>/dev/null > "${tmp_file}"
		src_rc="${?}"
		if [ "${src_rc}" = "0" ]
		then
			f_ipset "create"
			src_rc="${?}"
		elif [ "${ban_action}" != "refresh" ]
		then
			f_ipset "refresh"
			src_rc="${?}"
		fi
	else
		src_log="$(printf "%s" "${src_log}" | awk '{ORS=" ";print $0}')"
		if [ "${ban_action}" != "refresh" ]
		then
			f_ipset "refresh"
			src_rc="${?}"
		fi
		f_log "debug" "f_down  ::: name: ${src_name}, url: ${src_url}, rule: ${src_rule}, rc: ${src_rc}, log: ${src_log:-"-"}"
	fi
}

# main controller
#
f_main()
{
	local src_name src_url_4 src_rule_4 src_url_6 src_rule_6 src_comp src_rc src_ts log_raw log_merge log_ips log_count hold err_file cnt_file cnt=0

	# prepare logfile excerpts (dropbear, sshd, luci)
	#
	if [ "${ban_autoblacklist}" = "1" ] || [ "${ban_monitor_enabled}" = "1" ]
	then
		log_raw="$(${ban_logread_cmd} -l "${ban_loglimit}")"
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "dropbear")" ]
		then
			log_ips="$(printf "%s\n" "${log_raw}" | grep -E "Exit before auth from" | \
					awk 'match($0,/<[0-9A-f:\.]+:/){printf "%s\n",substr($0,RSTART+1,RLENGTH-2)}' | awk '!seen[$NF]++' | awk '{ORS=" ";print $NF}')"
			for ip in ${log_ips}
			do
				log_count="$(printf "%s\n" "${log_raw}" | grep -cE "Exit before auth from <${ip}")"
				if [ "${log_count}" -ge "${ban_ssh_logcount}" ]
				then
					log_merge="${log_merge} ${ip}"
				fi
			done
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "sshd")" ]
		then
			log_ips="$(printf "%s\n" "${log_raw}" | grep -E "error: maximum authentication attempts exceeded|sshd.*Connection closed by.*\[preauth\]" | \
					awk 'match($0,/[0-9A-f:\.]+ port/){printf "%s\n",substr($0,RSTART,RLENGTH-5)}' | awk '!seen[$NF]++' | awk '{ORS=" ";print $NF}')"
			for ip in ${log_ips}
			do
				log_count="$(printf "%s\n" "${log_raw}" | grep -cE "error: maximum authentication attempts exceeded.*${ip}|sshd.*Connection closed by.*${ip}.*\[preauth\]")"
				if [ "${log_count}" -ge "${ban_ssh_logcount}" ]
				then
					log_merge="${log_merge} ${ip}"
				fi
			done
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "luci")" ]
		then
			log_ips="$(printf "%s\n" "${log_raw}" | grep -E "luci: failed login on " | \
					awk 'match($0,/[0-9A-f:\.]+$/){printf "%s\n",substr($0,RSTART,RLENGTH)}' | awk '!seen[$NF]++' | awk '{ORS=" ";print $NF}')"
			for ip in ${log_ips}
			do
				log_count="$(printf "%s\n" "${log_raw}" | grep -cE "luci: failed login on .*from ${ip}")"
				if [ "${log_count}" -ge "${ban_luci_logcount}" ]
				then
					log_merge="${log_merge} ${ip}"
				fi
			done
		fi
		if [ -n "$(printf "%s\n" "${ban_logterms}" | grep -F "nginx")" ]
		then
			log_ips="$(printf "%s\n" "${log_raw}" | grep -oE "nginx\[[0-9]+\]:.*\[error\].*open().*client: [[:alnum:].:]+" | \
					awk '!seen[$NF]++' | awk '{ORS=" ";print $NF}')"
			for ip in ${log_ips}
			do
				log_count="$(printf "%s\n" "${log_raw}" | grep -cE "nginx\[[0-9]+\]:.*\[error\].*open().*client: ${ip}")"
				if [ "${log_count}" -ge "${ban_nginx_logcount}" ]
				then
					log_merge="${log_merge} ${ip}"
				fi
			done
		fi
	fi

	# prepare new black- and whitelist entries
	#
	if [ "${ban_autowhitelist}" = "1" ] && [ -f "${ban_whitelist}" ]
	then
		for ip in ${ban_subnets}
		do
			if [ -z "$(grep -F "${ip}" "${ban_whitelist}")" ]
			then
				src_ts="# added on $(date "+%d.%m.%Y %H:%M:%S")"
				printf "%-42s%s\n" "${ip}" "${src_ts}" >> "${ban_whitelist}"
				f_log "info" "IP address '${ip}' added to whitelist"
			fi
		done
	fi
	if [ "${ban_autoblacklist}" = "1" ] && [ -f "${ban_blacklist}" ]
	then
		for ip in ${log_merge}
		do
			if [ -z "$(grep -F "${ip}" "${ban_blacklist}")" ]
			then
				src_ts="# added on $(date "+%d.%m.%Y %H:%M:%S")"
				printf "%-42s%s\n" "${ip}" "${src_ts}" >> "${ban_blacklist}"
				f_log "info" "IP address '${ip}' added to blacklist"
			fi
		done
	fi

	# initial ipset/iptables creation
	#
	f_ipset "initial"
	if [ "${?}" != "0" ]
	then
		f_log "err" "banIP processing failed, fatal error during ipset/iptables creation (${ban_sysver})"
	fi

	# load local source files (maclist, blacklist, whitelist)
	#
	for src_name in ${ban_localsources}
	do
		if [ "${src_name}" = "maclist" ] && [ -s "${ban_maclist}" ]
		then
			(
				src_rule_4="/^([0-9A-z][0-9A-z]:){5}[0-9A-z]{2}([[:space:]]|$)/{print \"add ${src_name} \"toupper(\$1)}"
				f_down "${src_name}" "mac" "mac" "${ban_maclist}" "${src_rule_4}"
			)&
		fi
		if [ "${ban_proto4_enabled}" = "1" ]
		then
			if [ "${src_name}" = "blacklist" ] && [ -s "${ban_blacklist}" ] && [ "${ban_whitelistonly}" = "0" ]
			then
				(
					src_rule_4="/^(([0-9]{1,3}\\.){3}(1?[0-9][0-9]?|2[0-4][0-9]|25[0-5])(\\/(1?[0-9]|2?[0-9]|3?[0-2]))?)([[:space:]]|$)/{print \"add ${src_name}_4 \"\$1}"
					f_down "${src_name}" "4" "inet" "${ban_blacklist}" "${src_rule_4}"
				)&
			elif [ "${src_name}" = "whitelist" ] && [ -s "${ban_whitelist}" ]
			then
				(
					src_rule_4="/^(([0-9]{1,3}\\.){3}(1?[0-9][0-9]?|2[0-4][0-9]|25[0-5])(\\/(1?[0-9]|2?[0-9]|3?[0-2]))?)([[:space:]]|$)/{print \"add ${src_name}_4 \"\$1}"
					f_down "${src_name}" "4" "inet" "${ban_whitelist}" "${src_rule_4}"
				)&
			fi
		else
			(
				src_name="${src_name}_4"
				f_ipset "flush"
			)&
		fi
		if [ "${ban_proto6_enabled}" = "1" ]
		then
			if [ "${src_name}" = "blacklist" ] && [ -s "${ban_blacklist}" ] && [ "${ban_whitelistonly}" = "0" ]
			then
				(
					src_rule_6="/^(([0-9A-f]{0,4}:){1,7}[0-9A-f]{0,4}:?(\\/(1?[0-2][0-8]|[0-9][0-9]))?)([[:space:]]|$)/{print \"add ${src_name}_6 \"\$1}"
					f_down "${src_name}" "6" "inet6" "${ban_blacklist}" "${src_rule_6}"
				)&
			elif [ "${src_name}" = "whitelist" ] && [ -s "${ban_whitelist}" ]
			then
				(
					src_rule_6="/^(([0-9A-f]{0,4}:){1,7}[0-9A-f]{0,4}:?(\\/(1?[0-2][0-8]|[0-9][0-9]))?)([[:space:]]|$)/{print \"add ${src_name}_6 \"\$1}"
					f_down "${src_name}" "6" "inet6" "${ban_whitelist}" "${src_rule_6}"
				)&
			fi
		else
			(
				src_name="${src_name}_6"
				f_ipset "flush"
			)&
		fi
	done
	wait

	# loop over all external sources
	#
	if [ "${ban_whitelistonly}" = "0" ]
	then
		for src_name in ${ban_sources}
		do
			# get source data from JSON file
			#
			json_select "${src_name}" >/dev/null 2>&1
			if [ "${?}" != "0" ]
			then
				continue
			fi
			json_objects="url_4 rule_4 url_6 rule_6 comp"
			for object in ${json_objects}
			do
				eval json_get_var src_${object} "\${object}" >/dev/null 2>&1
			done
			json_select ..

			# handle external IPv4 source downloads in a subshell
			#
			if [ "${ban_proto4_enabled}" = "1" ] && [ -n "${src_url_4}" ] && [ -n "${src_rule_4}" ]
			then
				(
					f_down "${src_name}" "4" "inet" "${src_url_4}" "${src_rule_4}" "${src_comp}"
				)&
			fi

			# handle external IPv6 source downloads in a subshell
			#
			if [ "${ban_proto6_enabled}" = "1" ] && [ -n "${src_url_6}" ] && [ -n "${src_rule_6}" ]
			then
				(
					f_down "${src_name}" "6" "inet6" "${src_url_6}" "${src_rule_6}" "${src_comp}"
				)&
			fi

			# control/limit download queues
			#
			hold=$((cnt%ban_maxqueue))
			if [ "${hold}" = "0" ]
			then
				wait
			fi
			cnt=$((cnt+1))
		done
		wait
	fi

	# error out
	#
	for err_file in "${ban_tmpfile}."*".err"
	do
		if [ -f "${err_file}" ]
		then
			f_log "err" "banIP processing failed, fatal iptables errors during subshell processing (${ban_sysver})"
		fi
	done

	# finish processing
	#
	ban_sources=""
	for cnt_file in "${ban_tmpfile}."*".cnt"
	do
		if [ -f "${cnt_file}" ]
		then
			read -r cnt < "${cnt_file}"
			ban_cnt=$((ban_cnt+cnt))
			ban_setcnt=$((ban_setcnt+1))
			src_name="$(printf "%s" "${cnt_file}" | grep -Eo "[a-z0-9_]+.file.cnt")"
			src_name="${src_name%%.*}"
			if [ -z "$(printf "%s" "${ban_sources}" | grep -F "${src_name%_*}")" ]
			then
				ban_sources="${ban_sources} ${src_name%_*}"
				ban_allsources="${ban_allsources//${src_name%_*}/}"
			fi
		fi
	done
	for src_name in ${ban_allsources}
	do
		if [ "${src_name}" = "maclist" ]
		then
			f_ipset "flush"
		else
			for proto in "4" "6"
			do
				src_name="${src_name%_*}_${proto}"
				f_ipset "flush"
				if [ "${src_name%_*}" != "blacklist" ] && [ "${src_name%_*}" != "whitelist" ]
				then
					f_ipset "remove"
				fi
			done
		fi
	done
	f_log "info" "${ban_setcnt} IPSets with overall ${ban_cnt} IPs/Prefixes loaded successfully (${ban_sysver})"
	f_jsnup
	f_rmtmp
	f_bgsrv "start"
}

# query ipsets for certain IP
#
f_query()
{
	local src proto result query_start query_end query_timeout="30" match="0" search="${1}"

	if [ -z "${search}" ]
	then
		printf "%s\n" "::: missing search term, please submit a single ip or mac address :::"
	else
		query_start="$(date "+%s")"
		printf "%s\n%s\n%s\n" ":::" "::: search '${search}' in banIP related IPSets" ":::"

		for src in ${ban_localsources} ${ban_sources} ${ban_extrasources}
		do
			if [ "${src}" = "maclist" ] && [ -n "$("${ban_ipset_cmd}" -q -n list "${src}")" ]
			then
				result="$(ipset -q test ${src} ${search} >/dev/null 2>&1; printf "%u" "${?}")"
				if [ "${result}" = "0" ]
				then
					match="1"
					printf "%s\n" "  - found in IPSet '${src}'"
					break
				fi
			else
				for proto in "4" "6"
				do
					if [ -n "$("${ban_ipset_cmd}" -q -n list "${src}_${proto}")" ]
					then
						result="$(ipset -q test ${src}_${proto} ${search} >/dev/null 2>&1; printf "%u" "${?}")"
						if [ "${result}" = "0" ]
						then
							match="1"
							printf "%s\n" "  - found in IPSet '${src}_${proto}'"
						fi
					fi
				done
			fi
			query_end="$(date "+%s")"
			if [ "$((query_end-query_start))" -gt "${query_timeout}" ]
			then
				printf "%s\n\n" "  - [...]"
				break
			fi
		done
		if [ "${match}" = "0" ]
		then
			printf "%s\n\n" "  - no match"
		fi
	fi
}

# generate statistics
#
f_report()
{
	local report_json report_txt bg_pid content proto src src_list cnt cnt_mac cnt_cidr cnt_ip cnt_acc cnt_sum="0" cnt_set_sum="1" cnt_acc_sum="0" cnt_mac_sum="0" cnt_ip_sum="0" cnt_cidr_sum="0" cnt_set_sum="0" action="${1}"

	report_json="${ban_reportdir}/ban_report.json"
	report_txt="${ban_reportdir}/ban_mailreport.txt"

	# build json file
	#
	if [ "${action}" != "json" ] && { [ -n "$("${ban_ipt4_savecmd}" | grep " ${ban_chain} ")" ] || [ -n "$("${ban_ipt6_savecmd}" | grep " ${ban_chain} ")" ]; }
	then
		> "${report_json}"
		> "${report_txt}"
		printf "%s\n" "{" >> "${report_json}"
		printf "\t%s\n" "\"ipsets\": {" >> "${report_json}"
		for src in ${ban_localsources} ${ban_sources} ${ban_extrasources}
		do
			if [ -n "$(printf "%s" "${ban_extrasources}" | grep -F "${src}")" ]
			then
				set_type="n/a"
			else
				if [ -n "$(printf "%s\n" "${ban_settype_src}" | grep -F "${src}")" ]
				then
					set_type="src"
				elif [ -n "$(printf "%s\n" "${ban_settype_dst}" | grep -F "${src}")" ]
				then
					set_type="dst"
				elif [ -n "$(printf "%s\n" "${ban_settype_all}" | grep -F "${src}")" ]
				then
					set_type="src+dst"
				else
					set_type="${ban_global_settype}"
				fi
			fi
			if [ "${src}" = "maclist" ]
			then
				src_list="$("${ban_ipset_cmd}" -q list "${src}")"
				if [ -n "${src_list}" ]
				then
					cnt="$(printf "%s" "${src_list}" | awk '/^Number of entries:/{print $4}')"
					cnt_acc="$(printf "%s" "${src_list}" | grep -cE "packets [1-9]+")"
					cnt_acc_sum=$((cnt_acc_sum+cnt_acc))
					cnt_mac_sum="${cnt}"
					cnt_sum=$((cnt_sum+cnt))
					if [ "${cnt_set_sum}" != "0" ]
					then
						printf "%s\n" "," >> "${report_json}"
					fi
					printf "\t\t%s\n" "\"${src}\": {" >> "${report_json}"
					printf "\t\t\t%s\n" "\"type\": \"${set_type}\"," >> "${report_json}"
					printf "\t\t\t%s\n" "\"count\": \"${cnt}\"," >> "${report_json}"
					printf "\t\t\t%s\n" "\"count_ip\": \"0\"," >> "${report_json}"
					printf "\t\t\t%s\n" "\"count_cidr\": \"0\"," >> "${report_json}"
					printf "\t\t\t%s\n" "\"count_mac\": \"${cnt}\"," >> "${report_json}"
					printf "\t\t\t%s" "\"count_acc\": \"${cnt_acc}\"" >> "${report_json}"
					printf ",\n\t\t\t%s" "\"member_acc\": [" >> "${report_json}"
					printf "%s" "${src_list}" | awk 'match($0,/ packets [1-9]+/){printf "%s %s\n",$1,substr($0,RSTART+9,RLENGTH-9)}' | \
						awk 'BEGIN{i=0};{i=i+1;if(i==1){printf "\n\t\t\t\t\t{\n\t\t\t\t\t\t\"member\": \"%s\",\n\t\t\t\t\t\t\"packets\": \"%s\"\n\t\t\t\t\t}",$1,$2}else{printf ",\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\t\"member\": \"%s\",\n\t\t\t\t\t\t\t\"packets\": \"%s\"\n\t\t\t\t\t\t}",$1,$2}}' >> "${report_json}"
					printf "\n\t\t\t%s\n" "]" >> "${report_json}"
					printf "\t\t%s" "}" >> "${report_json}"
					cnt_set_sum=$((cnt_set_sum+1))
				fi
			else
				for proto in "4" "6"
				do
					src_list="$("${ban_ipset_cmd}" -q list "${src}_${proto}")"
					if [ -n "${src_list}" ]
					then
						cnt="$(printf "%s\n" "${src_list}" | awk '/^Number of entries:/{print $4}')"
						cnt_cidr="$(printf "%s\n" "${src_list}" | grep -cE "/[0-9]{1,3} ")"
						cnt_ip=$((cnt-cnt_cidr-cnt_mac))
						cnt_acc="$(printf "%s\n" "${src_list}" | grep -cE "packets [1-9]+")"
						cnt_cidr_sum=$((cnt_cidr_sum+cnt_cidr))
						cnt_ip_sum=$((cnt_ip_sum+cnt_ip))
						cnt_acc_sum=$((cnt_acc_sum+cnt_acc))
						cnt_sum=$((cnt_sum+cnt))
						if [ "${cnt_set_sum}" != "0" ]
						then
							printf "%s\n" "," >> "${report_json}"
						fi
						printf "\t\t%s\n" "\"${src}_${proto}\": {" >> "${report_json}"
						printf "\t\t\t%s\n" "\"type\": \"${set_type}\"," >> "${report_json}"
						printf "\t\t\t%s\n" "\"count\": \"${cnt}\"," >> "${report_json}"
						printf "\t\t\t%s\n" "\"count_ip\": \"${cnt_ip}\"," >> "${report_json}"
						printf "\t\t\t%s\n" "\"count_cidr\": \"${cnt_cidr}\"," >> "${report_json}"
						printf "\t\t\t%s\n" "\"count_mac\": \"0\"," >> "${report_json}"
						printf "\t\t\t%s" "\"count_acc\": \"${cnt_acc}\"" >> "${report_json}"
						printf ",\n\t\t\t%s" "\"member_acc\": [" >> "${report_json}"
						printf "%s" "${src_list}" | awk 'match($0,/ packets [1-9]+/){printf "%s %s\n",$1,substr($0,RSTART+9,RLENGTH-9)}' | \
							awk 'BEGIN{i=0};{i=i+1;if(i==1){printf "\n\t\t\t\t\t{\n\t\t\t\t\t\t\"member\": \"%s\",\n\t\t\t\t\t\t\"packets\": \"%s\"\n\t\t\t\t\t}",$1,$2}else{printf ",\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\t\"member\": \"%s\",\n\t\t\t\t\t\t\t\"packets\": \"%s\"\n\t\t\t\t\t\t}",$1,$2}}' >> "${report_json}"
						printf "\n\t\t\t%s\n" "]" >> "${report_json}"
						printf "\t\t%s" "}" >> "${report_json}"
						cnt_set_sum=$((cnt_set_sum+1))
					fi
				done
			fi
		done
		printf "\n\t%s" "}" >> "${report_json}"
		printf ",\n\t%s\n" "\"timestamp\": \"$(date "+%d.%m.%Y %H:%M:%S")\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_set_sum\": \"${cnt_set_sum}\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_ip_sum\": \"${cnt_ip_sum}\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_cidr_sum\": \"${cnt_cidr_sum}\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_mac_sum\": \"${cnt_mac_sum}\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_sum\": \"${cnt_sum}\"," >> "${report_json}"
		printf "\t%s\n" "\"cnt_acc_sum\": \"${cnt_acc_sum}\"" >> "${report_json}"
		printf "%s\n" "}" >> "${report_json}"
	fi

	# output preparation
	#
	if [ -s "${report_json}" ] && { [ "${action}" = "cli" ] || [ "${action}" = "mail" ]; }
	then
		printf "%s\n%s\n%s\n" ":::" "::: report on all banIP related IPSets" ":::" >> "${report_txt}"
		json_load_file "${report_json}" >/dev/null 2>&1
		json_get_var value "timestamp" >/dev/null 2>&1
		printf "  + %s\n" "Report timestamp           ::: ${value}" >> "${report_txt}"
		json_get_var value "cnt_set_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of all IPSets       ::: ${value:-"0"}" >> "${report_txt}"
		json_get_var value "cnt_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of all entries      ::: ${value:-"0"}" >> "${report_txt}"
		json_get_var value "cnt_ip_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of IP entries       ::: ${value:-"0"}" >> "${report_txt}"
		json_get_var value "cnt_cidr_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of CIDR entries     ::: ${value:-"0"}" >> "${report_txt}"
		json_get_var value "cnt_mac_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of MAC entries      ::: ${value:-"0"}" >> "${report_txt}"
		json_get_var value "cnt_acc_sum" >/dev/null 2>&1
		printf "  + %s\n" "Number of accessed entries ::: ${value:-"0"}" >> "${report_txt}"
		json_select "ipsets"
		json_get_keys ipsetlist
		if [ -n "${ipsetlist}" ]
		then
			printf "%s\n%s\n%s\n" ":::" "::: IPSet details" ":::" >> "${report_txt}"
			printf "%-25s%-12s%-11s%-10s%-10s%-10s%-10s%s\n" "    Name" "Type" "Count" "Cnt_IP" "Cnt_CIDR" "Cnt_MAC" "Cnt_ACC" "Entry details (Entry/Count)" >> "${report_txt}"
			printf "%s\n" "    --------------------------------------------------------------------------------------------------------------------" >> "${report_txt}"
		fi
		for ipset in ${ipsetlist}
		do
			set_info="${ipset}"
			acc_info=""
			json_select "${ipset}"
			json_get_keys detaillist
			for detail in ${detaillist}
			do
				if [ "${detail}" != "member_acc" ]
				then
					json_get_var value "${detail}" >/dev/null 2>&1
					set_info="${set_info} ${value}"
				elif [ "${detail}" = "member_acc" ]
				then
					index=1
					json_select "${detail}"
					while json_get_type type "${index}" && [ "${type}" = "object" ]
					do
						json_get_values values "${index}" >/dev/null 2>&1
						acc_info="${acc_info} ${values}"
						index=$((index+1))
					done
					json_select ".."
				fi
			done
			printf "    %-21s%-12s%-11s%-10s%-10s%-10s%s\n" ${set_info} >> "${report_txt}"
			if [ -n "${acc_info}" ]
			then
				printf "                                                                                        %-25s%s\n" ${acc_info} >> "${report_txt}"
			fi
			printf "%s\n" "    --------------------------------------------------------------------------------------------------------------------" >> "${report_txt}"
			json_select ".."
		done
		content="$(cat "${report_txt}" 2>/dev/null)"
		rm -f "${report_txt}"
	fi

	# report output
	#
	if [ "${action}" = "cli" ]
	then
		printf "%s\n" "${content}"
	elif [ "${action}" = "json" ]
	then
		cat "${ban_reportdir}/ban_report.json"
	elif [ "${action}" = "mail" ] && [ "${ban_mail_enabled}" = "1" ] && [ -x "${ban_mailservice}" ]
	then
		( "${ban_mailservice}" "${ban_ver}" "${content}" >/dev/null 2>&1 )&
		bg_pid="${!}"
	fi
	f_log "debug" "f_report ::: action: ${action}, report_json: ${report_json}, report_txt: ${report_txt}, bg_pid: ${bg_pid:-"-"}"
}

# update runtime information
#
f_jsnup()
{
	local memory entry runtime cnt_info status="${1:-"enabled"}"

	if [ "${status}" = "enabled" ] || [ "${status}" = "error" ]
	then
		ban_endtime="$(date "+%s")"
		cnt_info="${ban_setcnt} IPSets with ${ban_cnt} IPs/Prefixes"
		memory="$(awk '/^MemTotal|^MemFree|^MemAvailable/{ORS="/"; print int($2/1000)}' "/proc/meminfo" 2>/dev/null | awk '{print substr($0,1,length($0)-1)}')"
		if [ "$(( (ban_endtime-ban_starttime)/60 ))" -lt "60" ]
		then
			runtime="${ban_action}, $(( (ban_endtime-ban_starttime)/60 ))m $(( (ban_endtime-ban_starttime)%60 ))s, ${memory:-0}, $(date "+%d.%m.%Y %H:%M:%S")"
		else
			runtime="${ban_action}, n/a, ${memory:-0}, $(date "+%d.%m.%Y %H:%M:%S")"
		fi
	fi

	> "${ban_rtfile}"
	json_load_file "${ban_rtfile}" >/dev/null 2>&1
	json_init
	json_add_string "status" "${status}"
	json_add_string "version" "${ban_ver}"
	json_add_string "ipset_info" "${cnt_info:-"-"}"
	json_add_array "active_sources"
	if [ "${status}" = "running" ] || [ "${status}" = "error" ]
	then
		json_add_object
		json_add_string "source" "-"
		json_close_object
	else
		for entry in ${ban_sources}
		do
			json_add_object
			json_add_string "source" "${entry}"
			json_close_object
		done
	fi
	json_close_array
	json_add_array "active_devs"
	for entry in ${ban_devs}
	do
		json_add_object
		json_add_string "dev" "${entry}"
		json_close_object
	done
	json_close_array
	json_add_array "active_ifaces"
	for entry in ${ban_ifaces}
	do
		json_add_object
		json_add_string "iface" "${entry}"
		json_close_object
	done
	json_close_array
	json_add_array "active_logterms"
	for entry in ${ban_logterms}
	do
		json_add_object
		json_add_string "term" "${entry}"
		json_close_object
	done
	json_close_array
	json_add_array "active_subnets"
	for entry in ${ban_subnets}
	do
		json_add_object
		json_add_string "subnet" "${entry}"
		json_close_object
	done
	json_close_array
	json_add_string "run_infos" "settype: ${ban_global_settype}, backup_dir: ${ban_backupdir}, report_dir: ${ban_reportdir}"
	json_add_string "run_flags" "protocols (4/6): $(f_char ${ban_proto4_enabled})/$(f_char ${ban_proto6_enabled}), log (src/dst): $(f_char ${ban_logsrc_enabled})/$(f_char ${ban_logdst_enabled}), monitor: $(f_char ${ban_monitor_enabled}), mail: $(f_char ${ban_mail_enabled}), whitelist only: $(f_char ${ban_whitelistonly})"
	json_add_string "last_run" "${runtime:-"-"}"
	json_add_string "system" "${ban_sysver}"
	json_dump > "${ban_rtfile}"

	if [ "${ban_mail_enabled}" = "1" ] && [ -x "${ban_mailservice}" ] && { [ "${status}" = "error" ] || \
		{ [ "${status}" = "enabled" ] && { [ -z "${ban_mailactions}" ] || [ -n "$(printf "%s\n" "${ban_mailactions}" | grep -F "${ban_action}")" ]; }; }; }
	then
		( "${ban_mailservice}" "${ban_ver}" >/dev/null 2>&1 )&
		bg_pid="${!}"
	fi
	f_log "debug" "f_jsnup ::: status: ${status:-"-"}, action: ${ban_action}, mail_enabled: ${ban_mail_enabled}, mail_actions: ${ban_mailactions}, mail_service: ${ban_mailservice}, mail_pid: ${bg_pid:-"-"}"
}

# source required system libraries
#
if [ -r "/lib/functions.sh" ] && [ -r "/lib/functions/network.sh" ] && [ -r "/usr/share/libubox/jshn.sh" ]
then
	. "/lib/functions.sh"
	. "/lib/functions/network.sh"
	. "/usr/share/libubox/jshn.sh"
else
	f_log "err" "system libraries not found"
fi

if [ "${ban_action}" = "suspend" ] || [ "${ban_action}" = "resume" ] || \
	[ "${ban_action}" = "report" ] || [ "${ban_action}" = "query" ]
then
	json_load_file "${ban_rtfile}" >/dev/null 2>&1
	json_get_var ban_status "status"
fi

# version information
#
if [ "${ban_action}" = "version" ]
then
	printf "%s\n" "${ban_ver}"
	exit 0
fi

# handle different banIP actions
#
f_load
case "${ban_action}" in
	"stop")
		f_bgsrv "stop"
		f_ipset "destroy"
		f_jsnup "stopped"
		f_rmbckp
	;;
	"restart")
		f_ipset "destroy"
		f_rmbckp
		f_env
		f_main
	;;
	"suspend")
		if [ "${ban_status}" = "enabled" ] && [ "${ban_whitelistonly}" = "0" ]
		then
			f_bgsrv "stop"
			f_jsnup "running"
			f_ipset "suspend"
			f_jsnup "paused"
		fi
		f_rmtmp
	;;
	"resume")
		if [ "${ban_status}" = "paused" ] && [ "${ban_whitelistonly}" = "0" ]
		then
			f_env
			f_main
		else
			f_rmtmp
		fi
	;;
	"query")
		if [ "${ban_status}" = "enabled" ]
		then
			f_query "${2}"
		fi
	;;
	"report")
		if [ "${ban_status}" = "enabled" ] || [ "${2}" = "json" ]
		then
			f_report "${2}"
		fi
	;;
	"start"|"reload"|"refresh")
		f_env
		f_main
	;;
esac
