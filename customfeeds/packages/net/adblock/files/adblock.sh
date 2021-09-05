#!/bin/sh
# dns based ad/abuse domain blocking
# Copyright (c) 2015-2021 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the GNU General Public License v3.

# set (s)hellcheck exceptions
# shellcheck disable=1091,2010,2016,2034,2039,2059,2086,2091,2129,2143,2154,2181,2183,2188

# set initial defaults
#
export LC_ALL=C
export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
set -o pipefail
adb_ver="4.1.3"
adb_enabled=0
adb_debug=0
adb_forcedns=0
adb_maxqueue=4
adb_dnsflush=0
adb_dnstimeout=20
adb_safesearch=0
adb_safesearchlist=""
adb_safesearchmod=0
adb_report=0
adb_trigger=""
adb_triggerdelay=0
adb_backup=1
adb_mail=0
adb_mailcnt=0
adb_jail=0
adb_dns=""
adb_dnsprefix="adb_list"
adb_locallist="blacklist whitelist iplist"
adb_tmpbase="/tmp"
adb_backupdir="${adb_tmpbase}/adblock-Backup"
adb_reportdir="${adb_tmpbase}/adblock-Report"
adb_jaildir="/tmp"
adb_pidfile="/var/run/adblock.pid"
adb_blacklist="/etc/adblock/adblock.blacklist"
adb_whitelist="/etc/adblock/adblock.whitelist"
adb_mailservice="/etc/adblock/adblock.mail"
adb_dnsfile="${adb_dnsprefix}.overall"
adb_dnsjail="${adb_dnsprefix}.jail"
adb_srcarc="/etc/adblock/adblock.sources.gz"
adb_srcfile="${adb_tmpbase}/adb_sources.json"
adb_rtfile="${adb_tmpbase}/adb_runtime.json"
adb_loggercmd="$(command -v logger)"
adb_dumpcmd="$(command -v tcpdump)"
adb_lookupcmd="$(command -v nslookup)"
adb_fetchutil=""
adb_zonelist=""
adb_portlist=""
adb_repiface=""
adb_replisten="53"
adb_repchunkcnt="5"
adb_repchunksize="1"
adb_lookupdomain="example.com"
adb_action="${1:-"start"}"
adb_packages=""
adb_sources=""
adb_cnt=""

# load & check adblock environment
#
f_load()
{
	local bg_pid iface port ports

	adb_sysver="$(ubus -S call system board 2>/dev/null | jsonfilter -e '@.model' -e '@.release.description' | \
		"${adb_awk}" 'BEGIN{ORS=", "}{print $0}' | "${adb_awk}" '{print substr($0,1,length($0)-2)}')"
	f_conf
	if [ "${adb_action}" != "report" ]
	then
		f_dns
		f_fetch
	fi

	if [ "${adb_enabled}" = "0" ]
	then
		f_extconf
		f_temp
		f_rmdns
		f_jsnup "disabled"
		f_log "info" "adblock is currently disabled, please set the config option 'adb_enabled' to '1' to use this service"
		exit 0
	fi

	if [ "${adb_report}" = "1" ] && [ ! -x "${adb_dumpcmd}" ]
	then
		f_log "info" "Please install the package 'tcpdump' or 'tcpdump-mini' to use the reporting feature"
	elif [ "${adb_report}" = "0" ] && [ "${adb_action}" = "report" ]
	then
		f_log "info" "Please enable the 'DNS Report' option to use the reporting feature"
		exit 0
	fi

	bg_pid="$(pgrep -f "^${adb_dumpcmd}.*adb_report\\.pcap$" | "${adb_awk}" '{ORS=" "; print $1}')"
	if [ -x "${adb_dumpcmd}" ] && { [ "${adb_report}" = "0" ] || { [ -n "${bg_pid}" ] && { [ "${adb_action}" = "stop" ] || [ "${adb_action}" = "restart" ]; }; }; }
	then
		if [ -n "${bg_pid}" ]
		then
			kill -HUP "${bg_pid}" 2>/dev/null
			while $(kill -0 "${bg_pid}" 2>/dev/null)
			do
				sleep 1
			done
			unset bg_pid
		fi
	fi

	if [ -x "${adb_dumpcmd}" ] && [ "${adb_report}" = "1" ] && [ -z "${bg_pid}" ] && [ "${adb_action}" != "report" ] && [ "${adb_action}" != "stop" ]
	then
		for port in ${adb_replisten}
		do
			if [ -z "${ports}" ]
			then
				ports="port ${port}"
			else
				ports="${ports} or port ${port}"
			fi
		done
		if [ -z "${adb_repiface}" ]
		then
			network_get_device iface "lan"
			if [ -n "${iface}" ]
			then
				adb_repiface="${iface}"
			else
				network_get_physdev iface "lan"
				if [ -n "${iface}" ]
				then
					adb_repiface="${iface}"
				fi
			fi
			if [ -n "${adb_repiface}" ]
			then
				uci_set adblock global adb_repiface "${adb_repiface}"
				f_uci "adblock"
			fi
		fi
		if [ -n "${adb_reportdir}" ] && [ ! -d "${adb_reportdir}" ]
		then
			mkdir -p "${adb_reportdir}"
			f_log "info" "report directory '${adb_reportdir}' created"
		fi
		if [ -n "${adb_repiface}" ] && [ -d "${adb_reportdir}" ]
		then
			( "${adb_dumpcmd}" -nn -p -s0 -l -i ${adb_repiface} ${ports} -C${adb_repchunksize} -W${adb_repchunkcnt} -w "${adb_reportdir}/adb_report.pcap" >/dev/null 2>&1 & )
			bg_pid="$(pgrep -f "^${adb_dumpcmd}.*adb_report\\.pcap$" | "${adb_awk}" '{ORS=" "; print $1}')"
		else
			f_log "info" "Please set the name of the reporting network device 'adb_repiface' manually"
		fi
	fi
}

# check & set environment
#
f_env()
{
	adb_starttime="$(date "+%s")"
	f_log "info" "adblock instance started ::: action: ${adb_action}, priority: ${adb_nice:-"0"}, pid: ${$}"
	f_jsnup "running"
	f_extconf
	f_temp

	if [ "${adb_dnsflush}" = "1" ]
	then
		printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
		f_dnsup
	fi

	if [ ! -r "${adb_srcfile}" ]
	then
		if [ -r "${adb_srcarc}" ]
		then
			zcat "${adb_srcarc}" > "${adb_srcfile}"
		else
			f_log "err" "adblock source archive not found"
		fi
	fi
	if [ -r "${adb_srcfile}" ] && [ "${adb_action}" != "report" ]
	then
		json_init
		json_load_file "${adb_srcfile}"
	else
		f_log "err" "adblock source file not found"
	fi
}

# load adblock config
#
f_conf()
{
	local cnt=0 cnt_max=10

	if [ ! -r "/etc/config/adblock" ] || [ -z "$(uci -q show adblock.global.adb_safesearch)" ]
	then
		f_log "err" "no valid adblock config found, please re-install the package via opkg with the '--force-reinstall --force-maintainer' options"
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
			if [ "${option}" = "adb_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_sources}") ${value}\""
			elif [ "${option}" = "adb_eng_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_eng_sources}") ${value}\""
			elif [ "${option}" = "adb_sha_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_sha_sources}") ${value}\""
			elif [ "${option}" = "adb_stb_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_stb_sources}") ${value}\""
			elif [ "${option}" = "adb_utc_sources" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_utc_sources}") ${value}\""
			elif [ "${option}" = "adb_denyip" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_denyip}") ${value}\""
			elif [ "${option}" = "adb_allowip" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_allowip}") ${value}\""
			elif [ "${option}" = "adb_safesearchlist" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_safesearchlist}") ${value}\""
			elif [ "${option}" = "adb_zonelist" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_zonelist}") ${value}\""
			elif [ "${option}" = "adb_portlist" ]
			then
				eval "${option}=\"$(printf "%s" "${adb_portlist}") ${value}\""
			fi
		}
	}
	config_load adblock

	if [ -z "${adb_fetchutil}" ] || [ -z "${adb_dns}" ]
	then
		while [ -z "${adb_packages}" ] && [ "${cnt}" -le "${cnt_max}" ]
		do
			adb_packages="$(opkg list-installed 2>/dev/null)"
			cnt=$((cnt+1))
			sleep 1
		done
		if [ -z "${adb_packages}" ]
		then
			f_log "err" "local opkg package repository is not available, please set 'adb_fetchutil' and 'adb_dns' manually"
		fi
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

# load dns backend config
#
f_dns()
{
	local util utils dns_up cnt=0

	if [ -z "${adb_dns}" ]
	then
		utils="knot-resolver bind unbound dnsmasq raw"
		for util in ${utils}
		do
			if [ "${util}" = "raw" ] || [ -n "$(printf "%s" "${adb_packages}" | grep "^${util}")" ]
			then
				if [ "${util}" = "knot-resolver" ]
				then
					util="kresd"
				elif [ "${util}" = "bind" ]
				then
					util="named"
				fi
				if [ "${util}" = "raw" ] || [ -x "$(command -v "${util}")" ]
				then
					adb_dns="${util}"
					uci_set adblock global adb_dns "${util}"
					f_uci "adblock"
					break
				fi
			fi
		done
	elif [ "${adb_dns}" != "raw" ] && [ ! -x "$(command -v "${adb_dns}")" ]
	then
		unset adb_dns
	fi

	if [ -n "${adb_dns}" ]
	then
		case "${adb_dns}" in
			"dnsmasq")
				adb_dnscachecmd="-"
				adb_dnsinotify="${adb_dnsinotify:-"0"}"
				adb_dnsinstance="${adb_dnsinstance:-"0"}"
				adb_dnsuser="${adb_dnsuser:-"dnsmasq"}"
				adb_dnsdir="${adb_dnsdir:-"/tmp/dnsmasq.d"}"
				adb_dnsheader="${adb_dnsheader}"
				adb_dnsdeny="${adb_dnsdeny:-"${adb_awk} '{print \"address=/\"\$0\"/\"}'"}"
				adb_dnsallow="${adb_dnsallow:-"${adb_awk} '{print \"local=/\"\$0\"/#\"}'"}"
				adb_dnssafesearch="${adb_dnssafesearch:-"${adb_awk} -v item=\"\$item\" '{print \"address=/\"\$0\"/\"item\"\"}'"}"
				adb_dnsstop="${adb_dnsstop:-"address=/#/"}"
			;;
			"unbound")
				adb_dnscachecmd="$(command -v unbound-control || printf "%s" "-")"
				adb_dnsinotify="${adb_dnsinotify:-"0"}"
				adb_dnsinstance="${adb_dnsinstance:-"0"}"
				adb_dnsuser="${adb_dnsuser:-"unbound"}"
				adb_dnsdir="${adb_dnsdir:-"/var/lib/unbound"}"
				adb_dnsheader="${adb_dnsheader}"
				adb_dnsdeny="${adb_dnsdeny:-"${adb_awk} '{print \"local-zone: \\042\"\$0\"\\042 static\"}'"}"
				adb_dnsallow="${adb_dnsallow:-"${adb_awk} '{print \"local-zone: \\042\"\$0\"\\042 transparent\"}'"}"
				adb_dnssafesearch="${adb_dnssafesearch:-"${adb_awk} -v item=\"\$item\" '{type=\"AAAA\";if(match(item,/^([0-9]{1,3}\.){3}[0-9]{1,3}$/)){type=\"A\"}}{print \"local-data: \\042\"\$0\" \"type\" \"item\"\\042\"}'"}"
				adb_dnsstop="${adb_dnsstop:-"local-zone: \".\" static"}"
			;;
			"named")
				adb_dnscachecmd="$(command -v rndc || printf "%s" "-")"
				adb_dnsinotify="${adb_dnsinotify:-"0"}"
				adb_dnsinstance="${adb_dnsinstance:-"0"}"
				adb_dnsuser="${adb_dnsuser:-"bind"}"
				adb_dnsdir="${adb_dnsdir:-"/var/lib/bind"}"
				adb_dnsheader="${adb_dnsheader:-"\$TTL 2h\n@ IN SOA localhost. root.localhost. (1 6h 1h 1w 2h)\n  IN NS  localhost.\n"}"
				adb_dnsdeny="${adb_dnsdeny:-"${adb_awk} '{print \"\"\$0\" CNAME .\\n*.\"\$0\" CNAME .\"}'"}"
				adb_dnsallow="${adb_dnsallow:-"${adb_awk} '{print \"\"\$0\" CNAME rpz-passthru.\\n*.\"\$0\" CNAME rpz-passthru.\"}'"}"
				adb_dnsdenyip="${adb_dnsdenyip:-"${adb_awk} '{print \"\"\$0\".rpz-client-ip CNAME .\"}'"}"
				adb_dnsallowip="${adb_dnsallowip:-"${adb_awk} '{print \"\"\$0\".rpz-client-ip CNAME rpz-passthru.\"}'"}"
				adb_dnssafesearch="${adb_dnssafesearch:-"${adb_awk} -v item=\"\$item\" '{print \"\"\$0\" CNAME \"item\".\\n*.\"\$0\" CNAME \"item\".\"}'"}"
				adb_dnsstop="${adb_dnsstop:-"* CNAME ."}"
			;;
			"kresd")
				adb_dnscachecmd="-"
				adb_dnsinotify="${adb_dnsinotify:-"0"}"
				adb_dnsinstance="${adb_dnsinstance:-"0"}"
				adb_dnsuser="${adb_dnsuser:-"root"}"
				adb_dnsdir="${adb_dnsdir:-"/etc/kresd"}"
				adb_dnsheader="${adb_dnsheader:-"\$TTL 2h\n@ IN SOA localhost. root.localhost. (1 6h 1h 1w 2h)\n"}"
				adb_dnsdeny="${adb_dnsdeny:-"${adb_awk} '{print \"\"\$0\" CNAME .\\n*.\"\$0\" CNAME .\"}'"}"
				adb_dnsallow="${adb_dnsallow:-"${adb_awk} '{print \"\"\$0\" CNAME rpz-passthru.\\n*.\"\$0\" CNAME rpz-passthru.\"}'"}"
				adb_dnssafesearch="${adb_dnssafesearch:-"${adb_awk} -v item=\"\$item\" '{type=\"AAAA\";if(match(item,/^([0-9]{1,3}\.){3}[0-9]{1,3}$/)){type=\"A\"}}{print \"\"\$0\" \"type\" \"item\"\"}'"}"
				adb_dnsstop="${adb_dnsstop:-"* CNAME ."}"
			;;
			"raw")
				adb_dnscachecmd="-"
				adb_dnsinotify="${adb_dnsinotify:-"0"}"
				adb_dnsinstance="${adb_dnsinstance:-"0"}"
				adb_dnsuser="${adb_dnsuser:-"root"}"
				adb_dnsdir="${adb_dnsdir:-"/tmp"}"
				adb_dnsheader="${adb_dnsheader}"
				adb_dnsdeny="${adb_dnsdeny:-"0"}"
				adb_dnsallow="${adb_dnsallow:-"1"}"
				adb_dnssafesearch="${adb_dnssafesearch:-"0"}"
				adb_dnsstop="${adb_dnsstop:-"0"}"
			;;
		esac
	fi

	if [ "${adb_dns}" != "raw" ] && { [ -z "${adb_dns}" ] || [ ! -x "$(command -v "${adb_dns}")" ]; }
	then
		f_log "err" "dns backend not found, please set 'adb_dns' manually"
	fi

	if [ "${adb_dns}" != "raw" ] && { [ "${adb_dnsdir}" = "${adb_tmpbase}" ] || [ "${adb_dnsdir}" = "${adb_backupdir}" ] || [ "${adb_dnsdir}" = "${adb_reportdir}" ]; }
	then
		f_log "err" "dns directory '${adb_dnsdir}' has been misconfigured, it must not point to the 'adb_tmpbase', 'adb_backupdir', 'adb_reportdir'"
	fi

	if [ "${adb_action}" = "start" ] && [ -z "${adb_trigger}" ]
	then
		sleep ${adb_triggerdelay}
	fi

	if [ "${adb_dns}" != "raw" ] && [ "${adb_action}" != "stop" ]
	then
		while [ "${cnt}" -le 30 ]
		do
			dns_up="$(ubus -S call service list "{\"name\":\"${adb_dns}\"}" 2>/dev/null | jsonfilter -l1 -e "@[\"${adb_dns}\"].instances.*.running" 2>/dev/null)"
			if [ "${dns_up}" = "true" ]
			then
				break
			fi
			sleep 1
			cnt=$((cnt+1))
		done

		if [ -n "${adb_dnsdir}" ] && [ ! -d "${adb_dnsdir}" ]
		then
			mkdir -p "${adb_dnsdir}"
			if [ "${?}" = "0" ]
			then
				f_log "info" "dns backend directory '${adb_dnsdir}' created"
			else
				f_log "err" "dns backend directory '${adb_dnsdir}' could not be created"
			fi
		fi

		if [ ! -f "${adb_dnsdir}/${adb_dnsfile}" ]
		then
			printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
		fi

		if [ "${dns_up}" != "true" ]
		then
			f_dnsup 4
			if [ "${?}" != "0" ]
			then
				f_log "err" "dns backend '${adb_dns}' not running or executable"
			fi
		fi

		if [ "${adb_backup}" = "1" ] && [ -n "${adb_backupdir}" ] && [ ! -d "${adb_backupdir}" ]
		then
			mkdir -p "${adb_backupdir}"
			if [ "${?}" = "0" ]
			then
				f_log "info" "backup directory '${adb_backupdir}' created"
			else
				f_log "err" "backup directory '${adb_backupdir}' could not be created"
			fi
		fi

		if [ -n "${adb_jaildir}" ] && [ ! -d "${adb_jaildir}" ]
		then
			mkdir -p "${adb_jaildir}"
			if [ "${?}" = "0" ]
			then
				f_log "info" "jail directory '${adb_jaildir}' created"
			else
				f_log "err" "jail directory '${adb_jaildir}' could not be created"
			fi
		fi
	fi
	f_log "debug" "f_dns    ::: dns: ${adb_dns}, dns_dir: ${adb_dnsdir}, dns_file: ${adb_dnsfile}, dns_user: ${adb_dnsuser}, dns_inotify: ${adb_dnsinotify}, dns_instance: ${adb_dnsinstance}, backup: ${adb_backup}, backup_dir: ${adb_backupdir}, jail_dir: ${adb_jaildir}"
}

# load fetch utility
#
f_fetch()
{
	local util utils insecure cnt=0

	if [ -z "${adb_fetchutil}" ]
	then
		utils="aria2c curl wget uclient-fetch"
		for util in ${utils}
		do
			if { [ "${util}" = "uclient-fetch" ] && [ -n "$(printf "%s" "${adb_packages}" | grep "^libustream-")" ]; } || \
				{ [ "${util}" = "wget" ] && [ -n "$(printf "%s" "${adb_packages}" | grep "^wget -")" ]; } || \
				[ "${util}" = "curl" ] || [ "${util}" = "aria2c" ]
			then
				if [ -x "$(command -v "${util}")" ]
				then
					adb_fetchutil="${util}"
					uci_set adblock global adb_fetchutil "${util}"
					f_uci "adblock"
					break
				fi
			fi
		done
	elif [ ! -x "$(command -v "${adb_fetchutil}")" ]
	then
		unset adb_fetchutil
	fi
	case "${adb_fetchutil}" in
		"aria2c")
			if [ "${adb_fetchinsecure}" = "1" ]
			then
				insecure="--check-certificate=false"
			fi
			adb_fetchparm="${adb_fetchparm:-"${insecure} --timeout=20 --allow-overwrite=true --auto-file-renaming=false --log-level=warn --dir=/ -o"}"
		;;
		"curl")
			if [ "${adb_fetchinsecure}" = "1" ]
			then
				insecure="--insecure"
			fi
			adb_fetchparm="${adb_fetchparm:-"${insecure} --connect-timeout 20 --silent --show-error --location -o"}"
		;;
		"uclient-fetch")
			if [ "${adb_fetchinsecure}" = "1" ]
			then
				insecure="--no-check-certificate"
			fi
			adb_fetchparm="${adb_fetchparm:-"${insecure} --timeout=20 -O"}"
		;;
		"wget")
			if [ "${adb_fetchinsecure}" = "1" ]
			then
				insecure="--no-check-certificate"
			fi
			adb_fetchparm="${adb_fetchparm:-"${insecure} --no-cache --no-cookies --max-redirect=0 --timeout=20 -O"}"
		;;
	esac
	if [ -n "${adb_fetchutil}" ] && [ -n "${adb_fetchparm}" ]
	then
		adb_fetchutil="$(command -v "${adb_fetchutil}")"
	else
		f_log "err" "download utility with SSL support not found, please install 'uclient-fetch' with a 'libustream-*' variant or another download utility like 'wget', 'curl' or 'aria2'"
	fi
	f_log "debug" "f_fetch  ::: fetch_util: ${adb_fetchutil:-"-"}, fetch_parm: ${adb_fetchparm:-"-"}"
}

# create temporary files, directories and set dependent options
#
f_temp()
{
	local cpu core cores

	cpu="$(grep -c '^processor' /proc/cpuinfo 2>/dev/null)"
	core="$(grep -cm1 '^core id' /proc/cpuinfo 2>/dev/null)"
	if [ "${cpu}" = "0" ]
	then
		cpu=1
	fi
	if [ "${core}" = "0" ]
	then
		core=1
	fi
	cores=$((cpu*core))

	if [ -d "${adb_tmpbase}" ]
	then
		adb_tmpdir="$(mktemp -p "${adb_tmpbase}" -d)"
		adb_tmpload="$(mktemp -p "${adb_tmpdir}" -tu)"
		adb_tmpfile="$(mktemp -p "${adb_tmpdir}" -tu)"
		adb_srtopts="--temporary-directory=${adb_tmpdir} --compress-program=gzip --batch-size=32 --parallel=${cores}"
	else
		f_log "err" "the temp base directory '${adb_tmpbase}' does not exist/is not mounted yet, please create the directory or raise the 'adb_triggerdelay' to defer the adblock start"
	fi
	if [ ! -s "${adb_pidfile}" ]
	then
		printf "%s" "${$}" > "${adb_pidfile}"
	fi
	f_log "debug" "f_temp   ::: tmp_base: ${adb_tmpbase:-"-"}, tmp_dir: ${adb_tmpdir:-"-"}, cores: ${cores:-"-"}, sort_options: ${adb_srtopts}, pid_file: ${adb_pidfile:-"-"}"
}

# remove temporary files and directories
#
f_rmtemp()
{
	if [ -d "${adb_tmpdir}" ]
	then
		rm -rf "${adb_tmpdir}"
	fi
	rm -f "${adb_srcfile}"
	> "${adb_pidfile}"
	f_log "debug" "f_rmtemp ::: tmp_dir: ${adb_tmpdir:-"-"}, src_file: ${adb_srcfile:-"-"}, pid_file: ${adb_pidfile:-"-"}"
}

# remove dns related files
#
f_rmdns()
{
	local status

	status="$(ubus -S call service list '{"name":"adblock"}' 2>/dev/null | jsonfilter -l1 -e '@["adblock"].instances.*.running' 2>/dev/null)"
	if [ "${adb_dns}" = "raw" ] || { [ -n "${adb_dns}" ] && [ -n "${status}" ]; }
	then
		> "${adb_rtfile}"
		if [ "${adb_backup}" = "1" ]
		then
			rm -f "${adb_backupdir}/${adb_dnsprefix}".*.gz
		fi
		printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
		f_dnsup 4
	fi
	f_rmtemp
	f_log "debug" "f_rmdns  ::: dns: ${adb_dns}, status: ${status:-"-"}, dns_dir: ${adb_dnsdir}, dns_file: ${adb_dnsfile}, rt_file: ${adb_rtfile}, backup_dir: ${adb_backupdir:-"-"}"
}

# commit uci changes
#
f_uci()
{
	local change config="${1}"

	if [ -n "${config}" ]
	then
		change="$(uci -q changes "${config}" | "${adb_awk}" '{ORS=" "; print $0}')"
		if [ -n "${change}" ]
		then
			uci_commit "${config}"
			case "${config}" in
				"firewall")
					"/etc/init.d/firewall" reload >/dev/null 2>&1
				;;
				"resolver")
					printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
					f_count
					f_jsnup "running"
					"/etc/init.d/${adb_dns}" reload >/dev/null 2>&1
				;;
			esac
		fi
		f_log "debug" "f_uci    ::: config: ${config}, change: ${change}"
	fi
}

# get list counter
#
f_count()
{
	local file mode="${1}" name="${2}"

	adb_cnt=0
	case "${mode}" in
		"iplist")
			if [ -s "${adb_tmpdir}/tmp.add.${name}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_tmpdir}/tmp.add.${name}")"
			fi
		;;
		"blacklist")
			if [ -s "${adb_tmpfile}.${name}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_tmpfile}.${name}")"
			fi
		;;
		"whitelist")
			if [ -s "${adb_tmpdir}/tmp.raw.${name}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_tmpdir}/tmp.raw.${name}")"
				rm -f "${adb_tmpdir}/tmp.raw.${name}"
			fi
		;;
		"safesearch")
			if [ -s "${adb_tmpdir}/tmp.safesearch.${name}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_tmpdir}/tmp.safesearch.${name}")"
			fi
		;;
		"merge")
			if [ -s "${adb_tmpdir}/${adb_dnsfile}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_tmpdir}/${adb_dnsfile}")"
			fi
		;;
		"download"|"backup"|"restore")
			if [ -s "${src_tmpfile}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${src_tmpfile}")"
			fi
		;;
		"final")
			if [ -s "${adb_dnsdir}/${adb_dnsfile}" ]
			then
				adb_cnt="$(wc -l 2>/dev/null < "${adb_dnsdir}/${adb_dnsfile}")"
				if [ -s "${adb_tmpdir}/tmp.add.whitelist" ]
				then
					adb_cnt=$((adb_cnt-$(wc -l 2>/dev/null < "${adb_tmpdir}/tmp.add.whitelist")))
				fi
				for file in "${adb_tmpdir}/tmp.safesearch".*
				do
					if [ -r "${file}" ]
					then
						adb_cnt=$((adb_cnt-$(wc -l 2>/dev/null < "${file}")))
					fi
				done
				if [ -n "${adb_dnsheader}" ]
				then
					adb_cnt=$(((adb_cnt-$(printf "${adb_dnsheader}" | grep -c "^"))/2))
				fi
			fi
		;;
	esac
}

# set external config options
#
f_extconf()
{
	local config config_dir config_file section zone port fwcfg

	case "${adb_dns}" in
		"dnsmasq")
			config="dhcp"
			config_dir="$(uci_get dhcp "@dnsmasq[${adb_dnsinstance}]" confdir | grep -Fo "${adb_dnsdir}")"
			if [ "${adb_enabled}" = "1" ] && [ -z "${config_dir}" ]
			then
				uci_set dhcp "@dnsmasq[${adb_dnsinstance}]" confdir "${adb_dnsdir}" 2>/dev/null
			fi
		;;
		"kresd")
			config="resolver"
			config_file="$(uci_get resolver kresd rpz_file | grep -Fo "${adb_dnsdir}/${adb_dnsfile}")"
			if [ "${adb_enabled}" = "1" ] && [ -z "${config_file}" ]
			then
				uci -q add_list resolver.kresd.rpz_file="${adb_dnsdir}/${adb_dnsfile}"
			elif [ "${adb_enabled}" = "0" ] && [ -n "${config_file}" ]
			then
				uci -q del_list resolver.kresd.rpz_file="${adb_dnsdir}/${adb_dnsfile}"
			fi
		;;
	esac
	f_uci "${config}"

	config="firewall"
	fwcfg="$(uci -qNX show "${config}" | "${adb_awk}" 'BEGIN{FS="[.=]"};/adblock_/{if(zone==$2){next}else{ORS=" ";zone=$2;print zone}}')"
	if [ "${adb_enabled}" = "1" ] && [ "${adb_forcedns}" = "1" ] && \
		[ "$(/etc/init.d/firewall enabled; printf "%u" ${?})" = "0" ]
	then
		for zone in ${adb_zonelist}
		do
			for port in ${adb_portlist}
			do
				if [ -z "$(printf "%s" "${fwcfg}" | grep -o -m1 "adblock_${zone}${port}[ |\$]")" ]
				then
					uci -q batch <<-EOC
						set firewall."adblock_${zone}${port}"="redirect"
						set firewall."adblock_${zone}${port}".name="Adblock DNS (${zone}, ${port})"
						set firewall."adblock_${zone}${port}".src="${zone}"
						set firewall."adblock_${zone}${port}".proto="tcp udp"
						set firewall."adblock_${zone}${port}".src_dport="${port}"
						set firewall."adblock_${zone}${port}".dest_port="${port}"
						set firewall."adblock_${zone}${port}".target="DNAT"
					EOC
				fi
				fwcfg="${fwcfg/adblock_${zone}${port}[ |\$]/}"
			done
		done
		fwcfg="${fwcfg#"${fwcfg%%[![:space:]]*}"}"
		fwcfg="${fwcfg%"${fwcfg##*[![:space:]]}"}"
	fi
	if [ "${adb_enabled}" = "0" ] || [ "${adb_forcedns}" = "0" ] || [ -n "${fwcfg}" ]
	then
		for section in ${fwcfg}
		do
			uci_remove firewall "${section}"
		done
	fi
	f_uci "${config}"
}

# restart dns backend
#
f_dnsup()
{
	local dns_service dns_up dns_pid dns_procfile restart_rc cnt=0 out_rc=4 in_rc="${1:-0}"

	if [ "${adb_dns}" = "raw" ] || { [ "${in_rc}" = "0" ] && [ "${adb_dnsinotify}" = "1" ]; }
	then
		out_rc=0
	else
		if [ "${in_rc}" = "0" ] && [ "${adb_dnsflush}" = "0" ]
		then
			case "${adb_dns}" in
				"unbound")
					if [ -x "${adb_dnscachecmd}" ] && [ -d "${adb_tmpdir}" ] && [ -f "${adb_dnsdir}/unbound.conf" ]
					then
						"${adb_dnscachecmd}" -c "${adb_dnsdir}/unbound.conf" dump_cache > "${adb_tmpdir}/adb_cache.dump" 2>/dev/null
					fi
					"/etc/init.d/${adb_dns}" restart >/dev/null 2>&1
					restart_rc="${?}"
				;;
				"named")
					if [ -x "${adb_dnscachecmd}" ] && [ -f "/etc/bind/rndc.conf" ]
					then
						"${adb_dnscachecmd}" -c "/etc/bind/rndc.conf" reload >/dev/null 2>&1
						restart_rc="${?}"
					fi
					if [ -z "${restart_rc}" ] || { [ -n "${restart_rc}" ] && [ "${restart_rc}" != "0" ]; }
					then
						"/etc/init.d/${adb_dns}" restart >/dev/null 2>&1
						restart_rc="${?}"
					fi
				;;
				*)
					"/etc/init.d/${adb_dns}" restart >/dev/null 2>&1
					restart_rc="${?}"
				;;
			esac
		fi
		if [ -z "${restart_rc}" ]
		then
			"/etc/init.d/${adb_dns}" restart >/dev/null 2>&1
			restart_rc="${?}"
		fi
	fi
	if [ "${restart_rc}" = "0" ]
	then
		while [ "${cnt}" -le "${adb_dnstimeout}" ]
		do
			dns_service="$(ubus -S call service list "{\"name\":\"${adb_dns}\"}")"
			dns_up="$(printf "%s" "${dns_service}" | jsonfilter -l1 -e "@[\"${adb_dns}\"].instances.*.running")"
			dns_pid="$(printf "%s" "${dns_service}" | jsonfilter -l1 -e "@[\"${adb_dns}\"].instances.*.pid")"
			dns_procfile="$(ls -l "/proc/${dns_pid}/fd" 2>/dev/null | grep -Fo "${adb_dnsdir}/${adb_dnsfile}")"
			if [ "${dns_up}" = "true" ] && [ -n "${dns_pid}" ] && [ -z "${dns_procfile}" ]
			then
				if [ -x "${adb_lookupcmd}" ] && [ "${adb_lookupdomain}" != "false" ]
				then
					"${adb_lookupcmd}" "${adb_lookupdomain}" >/dev/null 2>&1
					if [ "${?}" = "0" ]
					then
						out_rc=0
						break
					fi
				else
					sleep ${adb_dnstimeout}
					cnt=${adb_dnstimeout}
					out_rc=0
					break
				fi
			fi
			cnt=$((cnt+1))
			sleep 1
		done
		if [ "${out_rc}" = "0" ] && [ "${adb_dns}" = "unbound" ]
		then
			if [ -x "${adb_dnscachecmd}" ] && [ -d "${adb_tmpdir}" ] && [ -s "${adb_tmpdir}/adb_cache.dump" ]
			then
				"${adb_dnscachecmd}" -c "${adb_dnsdir}/unbound.conf" load_cache < "${adb_tmpdir}/adb_cache.dump" >/dev/null 2>&1
				restart_rc="${?}"
			fi
		fi
	fi
	f_log "debug" "f_dnsup  ::: dns: ${adb_dns}, cache_cmd: ${adb_dnscachecmd:-"-"}, lookup_cmd: ${adb_lookupcmd:-"-"}, lookup_domain: ${adb_lookupdomain:-"-"}, restart_rc: ${restart_rc:-"-"}, dns_flush: ${adb_dnsflush}, dns_inotify: ${adb_dnsinotify}, dns_timeout: ${adb_dnstimeout}, dns_cnt: ${cnt}, in_rc: ${in_rc}, out_rc: ${out_rc}"
	return "${out_rc}"
}

# backup/restore/remove blocklists
#
f_list()
{
	local hold file rset item array safe_url safe_ips safe_cname safe_domains ip out_rc mode="${1}" src_name="${2:-"${src_name}"}" in_rc="${src_rc:-0}" cnt="1" ffiles="-maxdepth 1 -name ${adb_dnsprefix}.*.gz"

	case "${mode}" in
		"iplist")
			src_name="${mode}"
			if [ "${adb_dns}" = "named" ]
			then
				rset="BEGIN{FS=\"[.:]\";pfx=\"32\"}{if(match(\$0,/:/))pfx=\"128\"}{printf \"%s.\",pfx;for(seg=NF;seg>=1;seg--)if(seg==1)printf \"%s\n\",\$seg;else if(\$seg>=0)printf \"%s.\",\$seg; else printf \"%s.\",\"zz\"}"
				if [ -n "${adb_allowip}" ]
				then
					> "${adb_tmpdir}/tmp.raw.${src_name}"
					for ip in ${adb_allowip}
					do
						printf "%s" "${ip}" | "${adb_awk}" "${rset}" >> "${adb_tmpdir}/tmp.raw.${src_name}"
					done
					eval "${adb_dnsallowip}" "${adb_tmpdir}/tmp.raw.${src_name}" > "${adb_tmpdir}/tmp.add.${src_name}"
					out_rc="${?}"
				fi
				if [ -n "${adb_denyip}" ] && { [ -z "${out_rc}" ] || [ "${out_rc}" = "0" ]; }
				then
					> "${adb_tmpdir}/tmp.raw.${src_name}"
					for ip in ${adb_denyip}
					do
						printf "%s" "${ip}" | "${adb_awk}" "${rset}" >> "${adb_tmpdir}/tmp.raw.${src_name}"
					done
					eval "${adb_dnsdenyip}" "${adb_tmpdir}/tmp.raw.${src_name}" >> "${adb_tmpdir}/tmp.add.${src_name}"
					out_rc="${?}"
				fi
				rm -f "${adb_tmpdir}/tmp.raw.${src_name}"
			fi
		;;
		"blacklist"|"whitelist")
			src_name="${mode}"
			if [ "${src_name}" = "blacklist" ] && [ -f "${adb_blacklist}" ]
			then
				rset="/^([[:alnum:]_-]{1,63}\\.)+[[:alpha:]]+([[:space:]]|$)/{print tolower(\$1)}"
				"${adb_awk}" "${rset}" "${adb_blacklist}" | \
				"${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${adb_tmpdir}/tmp.raw.${src_name}"
				"${adb_sort}" ${adb_srtopts} -u "${adb_tmpdir}/tmp.raw.${src_name}" 2>/dev/null > "${adb_tmpfile}.${src_name}"
				out_rc="${?}"
				rm -f "${adb_tmpdir}/tmp.raw.${src_name}"
			elif [ "${src_name}" = "whitelist" ] && [ -f "${adb_whitelist}" ]
			then
				rset="/^([[:alnum:]_-]{1,63}\\.)+[[:alpha:]]+([[:space:]]|$)/{print tolower(\$1)}"
				"${adb_awk}" "${rset}" "${adb_whitelist}" > "${adb_tmpdir}/tmp.raw.${src_name}"
				out_rc="${?}"
				if [ "${out_rc}" = "0" ]
				then
					rset="/^([[:alnum:]_-]{1,63}\\.)+[[:alpha:]]+([[:space:]]|$)/{gsub(\"\\\\.\",\"\\\\.\",\$1);print tolower(\"^(|.*\\\\.)\"\$1\"$\")}"
					"${adb_awk}" "${rset}" "${adb_tmpdir}/tmp.raw.${src_name}" > "${adb_tmpdir}/tmp.rem.${src_name}"
					out_rc="${?}"
					if [ "${out_rc}" = "0" ] && [ "${adb_dnsallow}" != "1" ]
					then
						eval "${adb_dnsallow}" "${adb_tmpdir}/tmp.raw.${src_name}" > "${adb_tmpdir}/tmp.add.${src_name}"
						out_rc="${?}"
						if [ "${out_rc}" = "0" ] && [ "${adb_jail}" = "1" ] && [ "${adb_dnsstop}" != "0" ]
						then
							> "${adb_jaildir}/${adb_dnsjail}"
							if [ -n "${adb_dnsheader}" ]
							then
								printf "${adb_dnsheader}" >> "${adb_jaildir}/${adb_dnsjail}"
							fi
							cat "${adb_tmpdir}/tmp.add.${src_name}" >> "${adb_jaildir}/${adb_dnsjail}"
							printf "%s\n" "${adb_dnsstop}" >> "${adb_jaildir}/${adb_dnsjail}"
						fi
					fi
				fi
			fi
		;;
		"safesearch")
			case "${src_name}" in
				"google")
					rset="/^(\\.[[:alnum:]_-]{1,63}\\.)+[[:alpha:]]+([[:space:]]|$)/{printf \"%s\n%s\n\",tolower(\"www\"\$1),tolower(substr(\$1,2,length(\$1)))}"
					safe_url="https://www.google.com/supported_domains"
					safe_cname="forcesafesearch.google.com"
					safe_domains="${adb_tmpdir}/tmp.load.safesearch.${src_name}"
					if [ "${adb_backup}" = "1" ] && [ -s "${adb_backupdir}/safesearch.${src_name}.gz" ]
					then
						zcat "${adb_backupdir}/safesearch.${src_name}.gz" > "${safe_domains}"
						out_rc="${?}"
					else
						"${adb_fetchutil}" ${adb_fetchparm} "${safe_domains}" "${safe_url}" 2>/dev/null
						out_rc="${?}"
						if [ "${adb_backup}" = "1" ] && [ "${out_rc}" = "0" ]
						then
							gzip -cf "${safe_domains}" > "${adb_backupdir}/safesearch.${src_name}.gz"
						fi
					fi
					if [ "${out_rc}" = "0" ]
					then
						if [ -x "${adb_lookupcmd}" ]
						then
							safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
							if [ -n "${safe_ips}" ]
							then
								"${adb_awk}" "${rset}" "${safe_domains}" > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
							fi
						fi
						out_rc="${?}"
					fi
				;;
				"bing")
					safe_cname="strict.bing.com"
					safe_domains="www.bing.com"
					if [ -x "${adb_lookupcmd}" ]
					then
						safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
						if [ -n "${safe_ips}" ]
						then
							printf "%s\n" ${safe_domains} > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
						fi
					fi
					out_rc="${?}"
				;;
				"duckduckgo")
					safe_cname="safe.duckduckgo.com"
					safe_domains="duckduckgo.com"
					if [ -x "${adb_lookupcmd}" ]
					then
						safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
						if [ -n "${safe_ips}" ]
						then
							printf "%s\n" ${safe_domains} > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
						fi
					fi
					out_rc="${?}"
				;;
				"pixabay")
					safe_cname="safesearch.pixabay.com"
					safe_domains="pixabay.com"
					if [ -x "${adb_lookupcmd}" ]
					then
						safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
						if [ -n "${safe_ips}" ]
						then
							printf "%s\n" ${safe_domains} > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
						fi
					fi
					out_rc="${?}"
				;;
				"yandex")
					safe_cname="familysearch.yandex.ru"
					safe_domains="ya.ru yandex.ru yandex.com yandex.com.tr yandex.ua yandex.by yandex.ee yandex.lt yandex.lv yandex.md yandex.uz yandex.tm yandex.tj yandex.az yandex.kz"
					if [ -x "${adb_lookupcmd}" ]
					then
						safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
						if [ -n "${safe_ips}" ]
						then
							printf "%s\n" ${safe_domains} > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
						fi
					fi
					out_rc="${?}"
				;;
				"youtube")
					if [ "${adb_safesearchmod}" = "0" ]
					then
						safe_cname="restrict.youtube.com"
					else
						safe_cname="restrictmoderate.youtube.com"
					fi
					safe_domains="www.youtube.com m.youtube.com youtubei.googleapis.com youtube.googleapis.com www.youtube-nocookie.com"
					if [ -x "${adb_lookupcmd}" ]
					then
						safe_ips="$("${adb_lookupcmd}" "${safe_cname}" 2>/dev/null | "${adb_awk}" '/^Address[ 0-9]*: /{ORS=" ";print $NF}')"
						if [ -n "${safe_ips}" ]
						then
							printf "%s\n" ${safe_domains} > "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
						fi
					fi
					out_rc="${?}"
				;;
			esac
			if [ "${out_rc}" = "0" ] && [ -s "${adb_tmpdir}/tmp.raw.safesearch.${src_name}" ]
			then
				> "${adb_tmpdir}/tmp.safesearch.${src_name}"
				if [ "${adb_dns}" = "named" ]
				then
					array="${safe_cname}"
				else
					array="${safe_ips}"
				fi
				for item in ${array}
				do
					eval "${adb_dnssafesearch}" "${adb_tmpdir}/tmp.raw.safesearch.${src_name}" >> "${adb_tmpdir}/tmp.safesearch.${src_name}"
					if [ "${?}" != "0" ]
					then
						rm -f "${adb_tmpdir}/tmp.safesearch.${src_name}"
						break
					fi
				done
				out_rc="${?}"
				rm -f "${adb_tmpdir}/tmp.raw.safesearch.${src_name}"
			fi
		;;
		"backup")
			(
				gzip -cf "${src_tmpfile}" > "${adb_backupdir}/${adb_dnsprefix}.${src_name}.gz"
				out_rc="${?}"
			)&
		;;
		"restore")
			if [ -n "${src_name}" ] && [ -s "${adb_backupdir}/${adb_dnsprefix}.${src_name}.gz" ]
			then
				zcat "${adb_backupdir}/${adb_dnsprefix}.${src_name}.gz" > "${src_tmpfile}"
				out_rc="${?}"
			elif [ -z "${src_name}" ]
			then
				for file in "${adb_backupdir}/${adb_dnsprefix}".*.gz
				do
					if [ -r "${file}" ]
					then
						name="${file##*/}"
						name="${name%.*}"
						zcat "${file}" > "${adb_tmpfile}.${name}" &
						hold=$((cnt%adb_maxqueue))
						if [ "${hold}" = "0" ]
						then
							wait
						fi
						cnt=$((cnt+1))
					fi
				done
				wait
				out_rc="${?}"
			else
				out_rc=4
			fi
			if [ "${adb_action}" != "start" ] && [ "${adb_action}" != "resume" ] && [ -n "${src_name}" ] && [ "${out_rc}" != "0" ]
			then
				adb_sources="${adb_sources/${src_name}}"
			fi
		;;
		"remove")
			if [ "${adb_backup}" = "1" ]
			then
				rm "${adb_backupdir}/${adb_dnsprefix}.${src_name}.gz" 2>/dev/null
			fi
			out_rc="${?}"
			adb_sources="${adb_sources/${src_name}}"
		;;
		"merge")
			if [ "${adb_backup}" = "1" ]
			then
				for src_name in ${adb_sources}
				do
					ffiles="${ffiles} -a ! -name ${adb_dnsprefix}.${src_name}.gz"
				done
				if [ "${adb_safesearch}" = "1" ] && [ "${adb_dnssafesearch}" != "0" ]
				then
					ffiles="${ffiles} -a ! -name safesearch.google.gz"
				fi
				find "${adb_backupdir}" ${ffiles} -print0 2>/dev/null | xargs -0 rm 2>/dev/null
			fi
			unset src_name
			"${adb_sort}" ${adb_srtopts} -mu "${adb_tmpfile}".* 2>/dev/null > "${adb_tmpdir}/${adb_dnsfile}"
			out_rc="${?}"
			rm -f "${adb_tmpfile}".*
		;;
		"final")
			unset src_name
			if [ -n "${adb_dnsheader}" ]
			then
				printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
			else
				> "${adb_dnsdir}/${adb_dnsfile}"
			fi
			if [ -s "${adb_tmpdir}/tmp.add.iplist" ]
			then
				cat "${adb_tmpdir}/tmp.add.iplist" >> "${adb_dnsdir}/${adb_dnsfile}"
			fi
			if [ -s "${adb_tmpdir}/tmp.add.whitelist" ]
			then
				cat "${adb_tmpdir}/tmp.add.whitelist" >> "${adb_dnsdir}/${adb_dnsfile}"
			fi
			for file in "${adb_tmpdir}/tmp.safesearch".*
			do
				if [ -r "${file}" ]
				then
					cat "${file}" >> "${adb_dnsdir}/${adb_dnsfile}"
				fi
			done
			if [ "${adb_dnsdeny}" != "0" ]
			then
				eval "${adb_dnsdeny}" "${adb_tmpdir}/${adb_dnsfile}" >> "${adb_dnsdir}/${adb_dnsfile}"
			else
				mv "${adb_tmpdir}/${adb_dnsfile}" "${adb_dnsdir}/${adb_dnsfile}"
			fi
			out_rc="${?}"
		;;
	esac
	f_count "${mode}" "${src_name}"
	out_rc="${out_rc:-"${in_rc}"}"
	f_log "debug" "f_list   ::: name: ${src_name:-"-"}, mode: ${mode}, cnt: ${adb_cnt}, in_rc: ${in_rc}, out_rc: ${out_rc}"
	return "${out_rc}"
}

# top level domain compression
#
f_tld()
{
	local cnt cnt_tld source="${1}" temp_tld="${1}.tld"

	"${adb_awk}" '{if(NR==1){tld=$NF};while(getline){if(index($NF,tld".")==0){print tld;tld=$NF}}print tld}' "${source}" | \
	"${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${temp_tld}"
	if [ "${?}" = "0" ]
	then
		mv -f "${temp_tld}" "${source}"
		cnt_tld="$(wc -l 2>/dev/null < "${source}")"
	else
		rm -f "${temp_tld}"
	fi
	f_log "debug" "f_tld    ::: source: ${source}, cnt: ${adb_cnt:-"-"}, cnt_tld: ${cnt_tld:-"-"}"
}

# suspend/resume adblock processing
#
f_switch()
{
	local status entry done="false" mode="${1}"

	json_init
	json_load_file "${adb_rtfile}" >/dev/null 2>&1
	json_select "data" >/dev/null 2>&1
	json_get_var status "adblock_status"
	if [ "${mode}" = "suspend" ] && [ "${status}" = "enabled" ]
	then
		f_env
		printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
		if [ "${adb_jail}" = "1" ] && [ "${adb_jaildir}" = "${adb_dnsdir}" ]
		then
			printf "${adb_dnsheader}" > "${adb_jaildir}/${adb_dnsjail}"
		elif [ -f "${adb_dnsdir}/${adb_dnsjail}" ]
		then
			rm -f "${adb_dnsdir}/${adb_dnsjail}"
		fi
		f_count
		done="true"
	elif [ "${mode}" = "resume" ] && [ "${status}" = "paused" ]
	then
		f_env
		f_main
		done="true"
	fi
	if [ "${done}" = "true" ]
	then
		if [ "${mode}" = "suspend" ]
		then
			f_dnsup
		fi
		f_jsnup "${mode}"
		f_log "info" "${mode} adblock processing"
	fi
	f_rmtemp
}

# query blocklist for certain (sub-)domains
#
f_query()
{
	local search result prefix suffix field query_start query_end query_timeout=30 domain="${1}" tld="${1#*.}"

	if [ -z "${domain}" ] || [ "${domain}" = "${tld}" ]
	then
		printf "%s\n" "::: invalid input, please submit a single (sub-)domain :::"
	else
		case "${adb_dns}" in
			"dnsmasq")
				prefix=".*[\\/\\.]"
				suffix="(\\/)"
				field=2
			;;
			"unbound")
				prefix=".*[\"\\.]"
				suffix="(static)"
				field=3
			;;
			"named")
				prefix="[^\\*].*[\\.]"
				suffix="( \\.)"
				field=1
			;;
			"kresd")
				prefix="[^\\*].*[\\.]"
				suffix="( \\.)"
				field=1
			;;
			"raw")
				prefix=".*[\\.]"
				suffix=""
				field=1
			;;
		esac
		query_start="$(date "+%s")"
		while [ "${domain}" != "${tld}" ]
		do
			search="${domain//[+*~%\$&\"\']/}"
			search="${search//./\\.}"
			result="$("${adb_awk}" -F '/|\"|\t| ' "/^(${search}|${prefix}+${search}.*${suffix})$/{i++;if(i<=9){printf \"  + %s\n\",\$${field}}else if(i==10){printf \"  + %s\n\",\"[...]\";exit}}" "${adb_dnsdir}/${adb_dnsfile}")"
			printf "%s\n%s\n%s\n" ":::" "::: domain '${domain}' in active blocklist" ":::"
			printf "%s\n\n" "${result:-"  - no match"}"
			domain="${tld}"
			tld="${domain#*.}"
		done
		if [ "${adb_backup}" = "1" ] && [ -d "${adb_backupdir}" ]
		then
			search="${1//[+*~%\$&\"\']/}"
			search="${search//./\\.}"
			printf "%s\n%s\n%s\n" ":::" "::: domain '${1}' in backups and black-/whitelist" ":::"
			for file in "${adb_backupdir}/${adb_dnsprefix}".*.gz "${adb_blacklist}" "${adb_whitelist}"
			do
				suffix="${file##*.}"
				if [ "${suffix}" = "gz" ]
				then
					zcat "${file}" 2>/dev/null | \
					"${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' | "${adb_awk}" -v f="${file##*/}" "BEGIN{rc=1};/^($search|.*\\.${search})$/{i++;if(i<=3){printf \"  + %-30s%s\n\",f,\$1;rc=0}else if(i==4){printf \"  + %-30s%s\n\",f,\"[...]\"}};END{exit rc}"
				else
					"${adb_awk}" -v f="${file##*/}" "BEGIN{rc=1};/^($search|.*\\.${search})$/{i++;if(i<=3){printf \"  + %-30s%s\n\",f,\$1;rc=0}else if(i==4){printf \"  + %-30s%s\n\",f,\"[...]\"}};END{exit rc}" "${file}"
				fi
				if [ "${?}" = "0" ]
				then
					result="true"
					query_end="$(date "+%s")"
					if [ "$((query_end-query_start))" -gt "${query_timeout}" ]
					then
						printf "%s\n\n" "  - [...]"
						break
					fi
				fi
			done
			if [ "${result}" != "true" ]
			then
				printf "%s\n\n" "  - no match"
			fi
		fi
	fi
}

# update runtime information
#
f_jsnup()
{
	local entry sources runtime utils memory bg_pid status="${1:-"enabled"}"

	case "${status}" in
		"enabled"|"error")
			adb_endtime="$(date "+%s")"
			memory="$("${adb_awk}" '/^MemTotal|^MemFree|^MemAvailable/{ORS="/"; print int($2/1000)}' "/proc/meminfo" 2>/dev/null | "${adb_awk}" '{print substr($0,1,length($0)-1)}')"
			if [ "$(( (adb_endtime-adb_starttime)/60 ))" -lt 60 ]
			then
				runtime="${adb_action}, $(( (adb_endtime-adb_starttime)/60 ))m $(( (adb_endtime-adb_starttime)%60 ))s, ${memory:-0}, $(date "+%d.%m.%Y %H:%M:%S")"
			else
				runtime="${adb_action}, n/a, ${memory:-0}, $(date "+%d.%m.%Y %H:%M:%S")"
			fi
			if [ "${status}" = "error" ]
			then
				adb_cnt=0
			fi
		;;
		"suspend")
			status="paused"
		;;
		"resume")
			status=""
		;;
	esac
	json_init
	json_load_file "${adb_rtfile}" >/dev/null 2>&1
	if [ "${?}" = "0" ]
	then
		utils="download: $(readlink -fn "${adb_fetchutil}"), sort: $(readlink -fn "${adb_sort}"), awk: $(readlink -fn "${adb_awk}")"
		if [ -z "${adb_cnt}" ]
		then
			json_get_var adb_cnt "blocked_domains"
			adb_cnt="${adb_cnt%% *}"
		fi
		if [ -z "${runtime}" ]
		then
			json_get_var runtime "last_run"
		fi
	fi
	if [ "${adb_jail}" = "1" ] && [ "${adb_jaildir}" = "${adb_dnsdir}" ]
	then
		adb_cnt="0"
		sources="restrictive_jail"
	else
		sources="$(printf "%s\n" ${adb_sources} | "${adb_sort}" | "${adb_awk}" '{ORS=" ";print $0}')"
	fi

	> "${adb_rtfile}"
	json_init
	json_load_file "${adb_rtfile}" >/dev/null 2>&1
	json_init
	json_add_string "adblock_status" "${status:-"enabled"}"
	json_add_string "adblock_version" "${adb_ver}"
	json_add_string "blocked_domains" "${adb_cnt:-0}"
	json_add_array "active_sources"
	for entry in ${sources}
	do
		json_add_object
		json_add_string "source" "${entry}"
		json_close_object
	done
	json_close_array
	json_add_string "dns_backend" "${adb_dns:-"-"} (${adb_dnscachecmd##*/}), ${adb_dnsdir:-"-"}"
	json_add_string "run_utils" "${utils:-"-"}"
	json_add_string "run_ifaces" "trigger: ${adb_trigger:-"-"}, report: ${adb_repiface:-"-"}"	
	json_add_string "run_directories" "base: ${adb_tmpbase}, backup: ${adb_backupdir}, report: ${adb_reportdir}, jail: ${adb_jaildir}"
	json_add_string "run_flags" "backup: $(f_char ${adb_backup}), flush: $(f_char ${adb_dnsflush}), force: $(f_char ${adb_forcedns}), search: $(f_char ${adb_safesearch}), report: $(f_char ${adb_report}), mail: $(f_char ${adb_mail}), jail: $(f_char ${adb_jail})"
	json_add_string "last_run" "${runtime:-"-"}"
	json_add_string "system" "${adb_sysver}"
	json_dump > "${adb_rtfile}"

	if [ "${adb_mail}" = "1" ] && [ -x "${adb_mailservice}" ] && \
		{ [ "${status}" = "error" ] || { [ "${status}" = "enabled" ] && [ "${adb_cnt}" -le "${adb_mailcnt}" ]; } }
	then
		( "${adb_mailservice}" "${adb_ver}" >/dev/null 2>&1 )&
		bg_pid="${!}"
	fi
	f_log "debug" "f_jsnup  ::: status: ${status:-"-"}, cnt: ${adb_cnt}, mail: ${adb_mail}, mail_service: ${adb_mailservice}, mail_cnt: ${adb_mailcnt}, mail_pid: ${bg_pid:-"-"}"
}

# write to syslog
#
f_log()
{
	local class="${1}" log_msg="${2}"

	if [ -n "${log_msg}" ] && { [ "${class}" != "debug" ] || [ "${adb_debug}" = "1" ]; }
	then
		if [ -x "${adb_loggercmd}" ]
		then
			"${adb_loggercmd}" -p "${class}" -t "adblock-${adb_ver}[${$}]" "${log_msg}"
		else
			printf "%s %s %s\n" "${class}" "adblock-${adb_ver}[${$}]" "${log_msg}"
		fi
		if [ "${class}" = "err" ]
		then
			f_rmdns
			f_jsnup "error"
			exit 1
		fi
	fi
}

# main function for blocklist processing
#
f_main()
{
	local src_tmpload src_tmpfile src_name src_rset src_url src_log src_arc src_cat src_item src_list src_entries src_suffix src_rc entry memory cnt=1

	memory="$("${adb_awk}" '/^MemTotal|^MemFree|^MemAvailable/{ORS="/"; print int($2/1000)}' "/proc/meminfo" 2>/dev/null | "${adb_awk}" '{print substr($0,1,length($0)-1)}')"
	f_log "debug" "f_main   ::: memory: ${memory:-0}, max_queue: ${adb_maxqueue}, safe_search: ${adb_safesearch}, force_dns: ${adb_forcedns}, awk: ${adb_awk}"

	# white- and blacklist preparation
	#
	for entry in ${adb_locallist}
	do
		( f_list "${entry}" "${entry}" )&
	done

	if [ "${adb_dns}" != "raw" ] && [ "${adb_jail}" = "1" ] && [ "${adb_jaildir}" = "${adb_dnsdir}" ]
	then
		printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
		chown "${adb_dnsuser}" "${adb_jaildir}/${adb_dnsjail}" 2>/dev/null
		f_dnsup
		if [ "${?}" = "0" ]
		then
			if [ "${adb_action}" != "resume" ]
			then
				f_jsnup "enabled"
			fi
			f_log "info" "restrictive jail mode enabled successfully (${adb_sysver})"
		else
			f_log "err" "dns backend restart in jail mode failed"
		fi
		f_rmtemp
		return
	elif [ -f "${adb_dnsdir}/${adb_dnsjail}" ]
	then
		rm -f "${adb_dnsdir}/${adb_dnsjail}"
		f_dnsup
	fi

	# safe search preparation
	#
	if [ "${adb_safesearch}" = "1" ] && [ "${adb_dnssafesearch}" != "0" ]
	then
		if [ -z "${adb_safesearchlist}" ]
		then
			adb_safesearchlist="google bing duckduckgo pixabay yandex youtube"
		fi
		for entry in ${adb_safesearchlist}
		do
			( f_list safesearch "${entry}" )&
		done
	fi
	wait

	# main loop
	#
	for src_name in ${adb_sources}
	do
		json_select "${src_name}" >/dev/null 2>&1
		if [ "${?}" != "0" ]
		then
			adb_sources="${adb_sources/${src_name}}"
			continue
		fi
		json_get_var src_url "url" >/dev/null 2>&1
		json_get_var src_rset "rule" >/dev/null 2>&1
		json_select ..
		src_tmpcat="${adb_tmpload}.${src_name}.cat"
		src_tmpload="${adb_tmpload}.${src_name}.load"
		src_tmpsort="${adb_tmpload}.${src_name}.sort"
		src_tmpfile="${adb_tmpfile}.${src_name}"
		src_rc=4

		# basic pre-checks
		#
		if [ -z "${src_url}" ] || [ -z "${src_rset}" ]
		then
			f_list remove
			continue
		fi

		# backup mode
		#
		if [ "${adb_backup}" = "1" ] && { [ "${adb_action}" = "start" ] || [ "${adb_action}" = "resume" ]; }
		then
			f_list restore
			if [ "${?}" = "0" ] && [ -s "${src_tmpfile}" ]
			then
				continue
			fi
		fi

		# download queue processing
		#
		unset src_cat src_entries
		if [ "${src_name}" = "shallalist" ] || [ "${src_name}" = "utcapitole" ]
		then
			if [ "${src_name}" = "shallalist" ] && [ -n "${adb_sha_sources}" ]
			then
				src_cat="${adb_sha_sources}"
			elif [ "${src_name}" = "utcapitole" ] && [ -n "${adb_utc_sources}" ]
			then
				src_cat="${adb_utc_sources}"
			fi
			if [ -n "${src_cat}" ]
			then
				(
					src_arc="${adb_tmpdir}/${src_url##*/}"
					src_log="$("${adb_fetchutil}" ${adb_fetchparm} "${src_arc}" "${src_url}" 2>&1)"
					src_rc="${?}"
					if [ "${src_rc}" = "0" ] && [ -s "${src_arc}" ]
					then
						src_suffix="$(eval printf "%s" \"\$\{adb_src_suffix_${src_name}:-\"domains\"\}\")"
						src_list="$(tar -tzf "${src_arc}" 2>/dev/null)"
						for src_item in ${src_cat}
						do
							src_entries="${src_entries} $(printf "%s" "${src_list}" | grep -E "${src_item}/${src_suffix}$")"
						done
						if [ -n "${src_entries}" ]
						then
							tar -xOzf "${src_arc}" ${src_entries} 2>/dev/null > "${src_tmpload}"
							src_rc="${?}"
						fi
						> "${src_arc}"
					else
						src_log="$(printf "%s" "${src_log}" | "${adb_awk}" '{ORS=" ";print $0}')"
						f_log "info" "download of '${src_name}' failed, url: ${src_url}, rule: ${src_rset:-"-"}, categories: ${src_cat:-"-"}, rc: ${src_rc}, log: ${src_log:-"-"}"
					fi
					if [ "${src_rc}" = "0" ] && [ -s "${src_tmpload}" ]
					then
						if [ -s "${adb_tmpdir}/tmp.rem.whitelist" ]
						then
							"${adb_awk}" "${src_rset}" "${src_tmpload}" | sed "s/\r//g" | \
							grep -Evf "${adb_tmpdir}/tmp.rem.whitelist" | "${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${src_tmpsort}"
						else
							"${adb_awk}" "${src_rset}" "${src_tmpload}" | sed "s/\r//g" | \
							"${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${src_tmpsort}"
						fi
						> "${src_tmpload}"
						"${adb_sort}" ${adb_srtopts} -u "${src_tmpsort}" 2>/dev/null > "${src_tmpfile}"
						src_rc="${?}"
						> "${src_tmpsort}"
						if [ "${src_rc}" = "0" ] && [ -s "${src_tmpfile}" ]
						then
							f_list download
							if [ "${adb_backup}" = "1" ]
							then
								f_list backup
							fi
						elif [ "${adb_backup}" = "1" ] && [ "${adb_action}" != "start" ]
						then
							f_log "info" "archive preparation of '${src_name}' failed, categories: ${src_cat:-"-"}, entries: ${src_entries}, rc: ${src_rc}"
							f_list restore
							rm -f "${src_tmpfile}"
						fi
					elif [ "${adb_backup}" = "1" ] && [ "${adb_action}" != "start" ]
					then
						f_log "info" "archive extraction of '${src_name}' failed, categories: ${src_cat:-"-"}, entries: ${src_entries}, rc: ${src_rc}"
						f_list restore
					fi
				)&
			fi
		else
			if [ "${src_name}" = "energized" ] && [ -n "${adb_eng_sources}" ]
			then
				src_cat="${adb_eng_sources}"
			elif [ "${src_name}" = "stevenblack" ] && [ -n "${adb_stb_sources}" ]
			then
				src_cat="${adb_stb_sources}"
			elif { [ "${src_name}" = "energized" ] && [ -z "${adb_eng_sources}" ]; } || \
				{ [ "${src_name}" = "stevenblack" ] && [ -z "${adb_stb_sources}" ]; }
			then
				continue
			fi
			(
				for suffix in ${src_cat:-${src_url}}
				do
					if [ "${src_url}" != "${suffix}" ]
					then
						src_log="$("${adb_fetchutil}" ${adb_fetchparm} "${src_tmpcat}" "${src_url}${suffix}" 2>&1)"
						src_rc="${?}"
						if [ "${src_rc}" = "0" ] && [ -s "${src_tmpcat}" ]
						then
							cat "${src_tmpcat}" >> "${src_tmpload}"
							> "${src_tmpcat}"
						fi
					else
						src_log="$("${adb_fetchutil}" ${adb_fetchparm} "${src_tmpload}" "${src_url}" 2>&1)"
						src_rc="${?}"
					fi
				done
				if [ "${src_rc}" = "0" ] && [ -s "${src_tmpload}" ]
				then
					if [ -s "${adb_tmpdir}/tmp.rem.whitelist" ]
					then
						"${adb_awk}" "${src_rset}" "${src_tmpload}" | sed "s/\r//g" | \
						grep -Evf "${adb_tmpdir}/tmp.rem.whitelist" | "${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${src_tmpsort}"
					else
						"${adb_awk}" "${src_rset}" "${src_tmpload}" | sed "s/\r//g" | \
						"${adb_awk}" 'BEGIN{FS="."}{for(f=NF;f>1;f--)printf "%s.",$f;print $1}' > "${src_tmpsort}"
					fi
					> "${src_tmpload}"
					"${adb_sort}" ${adb_srtopts} -u "${src_tmpsort}" 2>/dev/null > "${src_tmpfile}"
					src_rc="${?}"
					> "${src_tmpsort}"
					if [ "${src_rc}" = "0" ] && [ -s "${src_tmpfile}" ]
					then
						f_list download
						if [ "${adb_backup}" = "1" ]
						then
							f_list backup
						fi
					elif [ "${adb_backup}" = "1" ] && [ "${adb_action}" != "start" ]
					then
						f_log "info" "preparation of '${src_name}' failed, rc: ${src_rc}"
						f_list restore
						rm -f "${src_tmpfile}"
					fi
				else
					src_log="$(printf "%s" "${src_log}" | "${adb_awk}" '{ORS=" ";print $0}')"
					f_log "info" "download of '${src_name}' failed, url: ${src_url}, rule: ${src_rset:-"-"}, categories: ${src_cat:-"-"}, rc: ${src_rc}, log: ${src_log:-"-"}"
					if [ "${adb_backup}" = "1" ] && [ "${adb_action}" != "start" ]
					then
						f_list restore
					fi
				fi
			)&
		fi
		hold=$((cnt%adb_maxqueue))
		if [ "${hold}" = "0" ]
		then
			wait
		fi
		cnt=$((cnt+1))
	done
	wait
	f_list merge

	# tld compression and dns restart
	#
	if [ "${?}" = "0" ] && [ -s "${adb_tmpdir}/${adb_dnsfile}" ]
	then
		f_tld "${adb_tmpdir}/${adb_dnsfile}"
		f_list final
	else
		printf "${adb_dnsheader}" > "${adb_dnsdir}/${adb_dnsfile}"
	fi
	chown "${adb_dnsuser}" "${adb_dnsdir}/${adb_dnsfile}" 2>/dev/null
	f_dnsup
	if [ "${?}" = "0" ]
	then
		if [ "${adb_action}" != "resume" ]
		then
			f_jsnup "enabled"
		fi
		f_log "info" "blocklist with overall ${adb_cnt} blocked domains loaded successfully (${adb_sysver})"
	else
		f_log "err" "dns backend restart with adblock blocklist failed"
	fi
	f_rmtemp
}

# trace dns queries via tcpdump and prepare a report
#
f_report()
{
	local report_raw report_json report_txt content status total start end blocked percent top_list top array item index hold ports value key key_list cnt=0 action="${1}" count="${2:-"50"}" search="${3:-"+"}"

	report_raw="${adb_reportdir}/adb_report.raw"
	report_srt="${adb_reportdir}/adb_report.srt"
	report_jsn="${adb_reportdir}/adb_report.json"
	report_txt="${adb_reportdir}/adb_mailreport.txt"

	# build json file
	#
	if [ "${action}" != "json" ]
	then
		> "${report_raw}"
		> "${report_srt}"
		> "${report_txt}"
		> "${report_jsn}"
		for file in "${adb_reportdir}/adb_report.pcap"*
		do
			(
				"${adb_dumpcmd}" -nn -tttt -r "${file}" 2>/dev/null | \
					"${adb_awk}" -v cnt="${cnt}" '!/\.lan\. |PTR\? | SOA\? /&&/ A[\? ]+|NXDomain|0\.0\.0\.0/{a=$1;b=substr($2,0,8);c=$4;sub(/\.[0-9]+$/,"",c);gsub(/[^[:alnum:]\.:-]/,"",c);d=cnt $7;sub(/\*$/,"",d);
					e=$(NF-1);sub(/[0-9]\/[0-9]\/[0-9]|0\.0\.0\.0/,"NX",e);sub(/\.$/,"",e);sub(/([0-9]{1,3}\.){3}[0-9]{1,3}/,"OK",e);gsub(/[^[:alnum:]\.-]/,"",e);if(e==""){e="err"};printf "%s\t%s\t%s\t%s\t%s\n",d,e,a,b,c}' >> "${report_raw}"
			)&
			hold=$((cnt%adb_maxqueue))
			if [ "${hold}" = "0" ]
			then
				wait
			fi
			cnt=$((cnt+1))
		done
		wait
		if [ -s "${report_raw}" ]
		then
			"${adb_sort}" ${adb_srtopts} -k1 -k3 -k4 -k5 -k1 -ur "${report_raw}" | \
				"${adb_awk}" '{currA=($1+0);currB=$1;currC=substr($1,length($1),1);if(reqA==currB){reqA=0;printf "%s\t%s\n",d,$2}else if(currC=="+"){reqA=currA;d=$3"\t"$4"\t"$5"\t"$2}}' | \
				"${adb_sort}" ${adb_srtopts} -k1 -k2 -k3 -k4 -ur > "${report_srt}"
			rm -f "${report_raw}"
		fi

		if [ -s "${report_srt}" ]
		then
			start="$("${adb_awk}" 'END{printf "%s_%s",$1,$2}' "${report_srt}")"
			end="$("${adb_awk}" 'NR==1{printf "%s_%s",$1,$2}' "${report_srt}")"
			total="$(wc -l < "${report_srt}")"
			blocked="$("${adb_awk}" '{if($5=="NX")cnt++}END{printf "%s",cnt}' "${report_srt}")"
			percent="$("${adb_awk}" -v t="${total}" -v b="${blocked}" 'BEGIN{printf "%.2f%s",b/t*100,"%"}')"
			> "${report_jsn}"
			printf "%s\n" "{ " >> "${report_jsn}"
			printf "\t%s\n" "\"start_date\": \"${start%_*}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"start_time\": \"${start#*_}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"end_date\": \"${end%_*}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"end_time\": \"${end#*_}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"total\": \"${total}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"blocked\": \"${blocked}\", " >> "${report_jsn}"
			printf "\t%s\n" "\"percent\": \"${percent}\", " >> "${report_jsn}"
			top_list="top_clients top_domains top_blocked"
			for top in ${top_list}
			do
				printf "\t%s" "\"${top}\": [ " >> "${report_jsn}"
				case "${top}" in
					"top_clients")
						"${adb_awk}" '{print $3}' "${report_srt}" | "${adb_sort}" ${adb_srtopts} | uniq -c | \
							"${adb_sort}" ${adb_srtopts} -nr | \
							"${adb_awk}" '{ORS=" ";if(NR==1)printf "\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2; else if(NR<10)printf ",\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2}' >> "${report_jsn}"
					;;
					"top_domains")
						"${adb_awk}" '{if($5!="NX")print $4}' "${report_srt}" | "${adb_sort}" ${adb_srtopts} | uniq -c | \
							"${adb_sort}" ${adb_srtopts} -nr | \
							"${adb_awk}" '{ORS=" ";if(NR==1)printf "\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2; else if(NR<10)printf ",\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2}' >> "${report_jsn}"
					;;
					"top_blocked")
						"${adb_awk}" '{if($5=="NX")print $4}' "${report_srt}" | \
							"${adb_sort}" ${adb_srtopts} | uniq -c | "${adb_sort}" ${adb_srtopts} -nr | \
							"${adb_awk}" '{ORS=" ";if(NR==1)printf "\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2; else if(NR<10)printf ",\n\t\t{\n\t\t\t\"count\": \"%s\",\n\t\t\t\"address\": \"%s\"\n\t\t}",$1,$2}' >> "${report_jsn}"
					;;
				esac
				printf "\n\t%s\n" "]," >> "${report_jsn}"
			done
			search="${search//./\\.}"
			search="${search//[+*~%\$&\"\' ]/}"
			"${adb_awk}" "BEGIN{i=0;printf \"\t\\\"requests\\\": [\n\"}/(${search})/{i++;if(i==1)printf \"\n\t\t{\n\t\t\t\\\"date\\\": \\\"%s\\\",\n\t\t\t\\\"time\\\": \\\"%s\\\",\n\t\t\t\\\"client\\\": \\\"%s\\\",\n\t\t\t\\\"domain\\\": \\\"%s\\\",\n\t\t\t\\\"rc\\\": \\\"%s\\\"\n\t\t}\",\$1,\$2,\$3,\$4,\$5;else if(i<=${count})printf \",\n\t\t{\n\t\t\t\\\"date\\\": \\\"%s\\\",\n\t\t\t\\\"time\\\": \\\"%s\\\",\n\t\t\t\\\"client\\\": \\\"%s\\\",\n\t\t\t\\\"domain\\\": \\\"%s\\\",\n\t\t\t\\\"rc\\\": \\\"%s\\\"\n\t\t}\",\$1,\$2,\$3,\$4,\$5}END{printf \"\n\t]\n}\n\"}" "${adb_reportdir}/adb_report.srt" >> "${report_jsn}"
			rm -f "${report_srt}"
		fi
	fi

	# output preparation
	#
	if [ -s "${report_jsn}" ] && { [ "${action}" = "cli" ] || [ "${action}" = "mail" ]; }
	then
		printf "%s\n%s\n%s\n" ":::" "::: Adblock DNS-Query Report" ":::" >> "${report_txt}"
		json_init
		json_load_file "${report_jsn}"
		json_get_keys key_list
		for key in ${key_list}
		do
			json_get_var value "${key}"
			eval "${key}=\"${value}\""
		done
		printf "  + %s\n  + %s\n" "Start    ::: ${start_date}, ${start_time}" "End      ::: ${end_date}, ${end_time}" >> "${report_txt}"
		printf "  + %s\n  + %s %s\n" "Total    ::: ${total}" "Blocked  ::: ${blocked}" "(${percent})" >> "${report_txt}"
		top_list="top_clients top_domains top_blocked requests"
		for top in ${top_list}
		do
			case "${top}" in
				"top_clients")
					item="::: Top 10 Clients"
				;;
				"top_domains")
					item="::: Top 10 Domains"
				;;
				"top_blocked")
					item="::: Top 10 Blocked Domains"
				;;
			esac
			if json_get_type status "${top}" && [ "${top}" != "requests" ] && [ "${status}" = "array" ]
			then
				printf "%s\n%s\n%s\n" ":::" "${item}" ":::" >> "${report_txt}"
				json_select "${top}"
				index=1
				item=""
				while json_get_type status "${index}" && [ "${status}" = "object" ]
				do
					json_get_values item "${index}"
					printf "  + %-9s::: %s\n" ${item} >> "${report_txt}"
					index=$((index+1))
				done
			elif json_get_type status "${top}" && [ "${top}" = "requests" ] && [ "${status}" = "array" ]
			then
				printf "%s\n%s\n%s\n" ":::" "::: Latest DNS Queries" ":::" >> "${report_txt}"
				printf "%-15s%-15s%-45s%-80s%s\n" "Date" "Time" "Client" "Domain" "Answer" >> "${report_txt}"
				json_select "${top}"
				index=1
				while json_get_type status "${index}" && [ "${status}" = "object" ]
				do
					json_get_values item "${index}"
					printf "%-15s%-15s%-45s%-80s%s\n" ${item} >> "${report_txt}"
					index=$((index+1))
				done
			fi
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
		cat "${report_jsn}"
	elif [ "${action}" = "mail" ] && [ "${adb_mail}" = "1" ] && [ -x "${adb_mailservice}" ]
	then
		( "${adb_mailservice}" "${adb_ver}" "${content}" >/dev/null 2>&1 )&
		bg_pid="${!}"
	fi
	f_log "debug" "f_report ::: action: ${action}, count: ${count}, search: ${search}, dump_util: ${adb_dumpcmd}, rep_dir: ${adb_reportdir}, rep_iface: ${adb_repiface:-"-"}, rep_listen: ${adb_replisten}, rep_chunksize: ${adb_repchunksize}, rep_chunkcnt: ${adb_repchunkcnt}"
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

# version information
#
if [ "${adb_action}" = "version" ]
then
	printf "%s\n" "${adb_ver}"
	exit 0
fi

# awk check
#
adb_awk="$(command -v gawk)"
if [ ! -x "${adb_awk}" ]
then
	adb_awk="$(command -v awk)"
	if [ ! -x "${adb_awk}" ]
	then
		f_log "err" "awk not found or not executable"
	fi
fi

# sort check
#
adb_sort="$(command -v sort)"
if [ ! -x "${adb_sort}" ] || [ "$("${adb_sort}" --version 2>/dev/null | grep -c "coreutils")" = "0" ]
then
	f_log "err" "coreutils sort not found or not executable"
fi

# handle different adblock actions
#
f_load
case "${adb_action}" in
	"stop")
		f_rmdns
	;;
	"restart")
		f_rmdns
		f_env
		f_main
	;;
	"suspend")
		if [ "${adb_dns}" != "raw" ]
		then
			f_switch suspend
		fi
	;;
	"resume")
		if [ "${adb_dns}" != "raw" ]
		then
			f_switch resume
		fi
	;;
	"report")
		f_report "${2}" "${3}" "${4}"
	;;
	"query")
		f_query "${2}"
	;;
	"start"|"reload")
		f_env
		f_main
	;;
esac
