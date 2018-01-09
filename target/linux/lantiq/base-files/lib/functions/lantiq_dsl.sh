#!/bin/sh /etc/rc.common
# Copyright (C) 2012-2014 OpenWrt.org

if [ "$( which vdsl_cpe_control )" ]; then
	XDSL_CTRL=vdsl_cpe_control
else
	XDSL_CTRL=dsl_cpe_control
fi

#
# Basic functions to send CLI commands to the vdsl_cpe_control daemon
#
dsl_cmd() {
	killall -q -0 ${XDSL_CTRL} && (
		lock /var/lock/dsl_pipe
		echo "$@" > /tmp/pipe/dsl_cpe0_cmd
		cat /tmp/pipe/dsl_cpe0_ack
		lock -u /var/lock/dsl_pipe
	)
}
dsl_val() {
	echo $(expr "$1" : '.*'$2'=\([-\.[:alnum:]]*\).*')
}
dsl_string() {
	echo $(expr "$1" : '.*'$2'=(\([A-Z0-9,]*\))')
}

#
# Simple divide by 10 routine to cope with one decimal place
#
dbt() {
	local a=$(expr $1 / 10)
	local b=$(expr $1 % 10)
	echo "${a}.${b#-}"
}
#
# Take a number and convert to k or meg
#
scale() {
	local val=$1
	local a
	local b

	if [ "$val" -gt 1000000 ]; then
		a=$(expr $val / 1000)
		b=$(expr $a % 1000)
		a=$(expr $a / 1000)
		printf "%d.%03d Mb" ${a} ${b}
	elif [ "$val" -gt 1000 ]; then
		a=$(expr $val / 1000)
		printf "%d Kb" ${a}
	else
		echo "${val} b"
	fi
}

scale_latency() {
	local val=$1
	local a
	local b

	a=$(expr $val / 100)
	b=$(expr $val % 100)
	printf "%d.%d ms" ${a} ${b}
}

#
# convert vendorid into human readable form
#
parse_vendorid() {
	local val=$1
	local name
	local version

	case "$val" in
		B5,00,41,4C,43,42*)
			name="Alcatel"
			version=${val##*B5,00,41,4C,43,42,}
			;;
		B5,00,41,4E,44,56*)
			name="Analog Devices"
			version=${val##*B5,00,41,4E,44,56,}
			;;
		B5,00,42,44,43,4D*)
			name="Broadcom"
			version=${val##*B5,00,42,44,43,4D,}
			;;
		B5,00,43,45,4E,54*)
			name="Centillium"
			version=${val##*B5,00,43,45,4E,54,}
			;;
		B5,00,47,53,50,4E*)
			name="Globespan"
			version=${val##*B5,00,47,53,50,4E,}
			;;
		B5,00,49,4B,4E,53*)
			name="Ikanos"
			version=${val##*B5,00,49,4B,4E,53,}
			;;
		B5,00,49,46,54,4E*)
			name="Infineon"
			version=${val##*B5,00,49,46,54,4E,}
			;;
		B5,00,54,53,54,43*)
			name="Texas Instruments"
			version=${val##*B5,00,54,53,54,43,}
			;;
		B5,00,54,4D,4D,42*)
			name="Thomson MultiMedia Broadband"
			version=${val##*B5,00,54,4D,4D,42,}
			;;
		B5,00,54,43,54,4E*)
			name="Trend Chip Technologies"
			version=${val##*B5,00,54,43,54,4E,}
			;;
		B5,00,53,54,4D,49*)
			name="ST Micro"
			version=${val##*B5,00,53,54,4D,49,}
			;;
	esac

	[ -n "$name" ] && {
		val="$name"

		[ "$version" != "00,00" ] && val="$(printf "%s %d.%d" "$val" 0x${version//,/ 0x})"
	}

	echo "$val"
}

#
# Read the data rates for both directions
#
data_rates() {
	local csg
	local dru
	local drd
	local sdru
	local sdrd

	csg=$(dsl_cmd g997csg 0 1)
	drd=$(dsl_val "$csg" ActualDataRate)

	csg=$(dsl_cmd g997csg 0 0)
	dru=$(dsl_val "$csg" ActualDataRate)

	[ -z "$drd" ] && drd=0
	[ -z "$dru" ] && dru=0

	sdrd=$(scale $drd)
	sdru=$(scale $dru)

	if [ "$action" = "lucistat" ]; then
		echo "dsl.data_rate_down=$drd"
		echo "dsl.data_rate_up=$dru"
		echo "dsl.data_rate_down_s=\"$sdrd\""
		echo "dsl.data_rate_up_s=\"$sdru\""
	else
		echo "Data Rate:                                Down: ${sdrd}/s / Up: ${sdru}/s"
	fi
}

#
# Chipset
#
chipset() {
	local vig
	local cs

	vig=$(dsl_cmd vig)
	cs=$(dsl_val "$vig" DSL_ChipSetType)
	csfw=$(dsl_val "$vig" DSL_ChipSetFWVersion)
	csapi=$(dsl_val "$vig" DSL_DriverVersionApi)

	if [ "$action" = "lucistat" ]; then
		echo "dsl.chipset=\"${cs}\""
		echo "dsl.firmware_version=\"${csfw}\""
		echo "dsl.api_version=\"${csapi}\""
	else
		echo "Chipset:                                  ${cs}"
		echo "Firmware Version:                         ${csfw}"
		echo "API Version:                              ${csapi}"
	fi
}

#
# Vendor information
#
vendor() {
	local lig
	local vid
	local svid

	lig=$(dsl_cmd g997lig 1)
	vid=$(dsl_string "$lig" G994VendorID)
	svid=$(dsl_string "$lig" SystemVendorID)

	vid=$(parse_vendorid $vid)
	svid=$(parse_vendorid $svid)

	if [ "$action" = "lucistat" ]; then
		echo "dsl.atuc_vendor_id=\"${vid}\""
		echo "dsl.atuc_system_vendor_id=\"${svid}\""
	else
		echo "ATU-C Vendor ID:                          ${vid}"
		echo "ATU-C System Vendor ID:                   ${svid}"
	fi
}

#
# XTSE capabilities
#
xtse() {
	local xtusesg
	local xtse1
	local xtse2
	local xtse3
	local xtse4
	local xtse5
	local xtse6
	local xtse7
	local xtse8

	local xtse_s=""

	local annex_s=""
	local line_mode_s=""
	local vector_s=""

	local dsmsg=""
	local cmd=""

	xtusesg=$(dsl_cmd g997xtusesg)
	xtse1=$(dsl_val "$xtusesg" XTSE1)
	xtse2=$(dsl_val "$xtusesg" XTSE2)
	xtse3=$(dsl_val "$xtusesg" XTSE3)
	xtse4=$(dsl_val "$xtusesg" XTSE4)
	xtse5=$(dsl_val "$xtusesg" XTSE5)
	xtse6=$(dsl_val "$xtusesg" XTSE6)
	xtse7=$(dsl_val "$xtusesg" XTSE7)
	xtse8=$(dsl_val "$xtusesg" XTSE8)

	# Evaluate Annex (according to G.997.1, 7.3.1.1.1)
	if [ $((xtse1 & 13)) != 0 \
	-o $((xtse2 & 1)) != 0 \
	-o $((xtse3 & 12)) != 0 \
	-o $((xtse4 & 3)) != 0 \
	-o $((xtse6 & 3)) != 0 \
	-o $((xtse8 & 1)) != 0 ]; then
		annex_s=" A,"
	fi

	if [ $((xtse1 & 48)) != 0 \
	-o $((xtse2 & 2)) != 0 \
	-o $((xtse3 & 48)) != 0 \
	-o $((xtse6 & 12)) != 0 \
	-o $((xtse8 & 2)) != 0 ]; then
		annex_s="$annex_s B,"
	fi

	if [ $((xtse1 & 194)) != 0 \
	-o $((xtse2 & 12)) != 0 \
	-o $((xtse8 & 4)) != 0 ]; then
		annex_s="$annex_s C,"
	fi

	if [ $((xtse4 & 48)) != 0 \
	-o $((xtse5 & 3)) != 0 \
	-o $((xtse6 & 192)) != 0 ]; then
		annex_s="$annex_s I,"
	fi

	if [ $((xtse4 & 192)) != 0 \
	-o $((xtse7 & 3)) != 0 ]; then
		annex_s="$annex_s J,"
	fi

	if [ $((xtse5 & 60)) != 0 ]; then
		annex_s="$annex_s L,"
	fi

	if [ $((xtse5 & 192)) != 0 \
	-o $((xtse7 & 12)) != 0 ]; then
		annex_s="$annex_s M,"
	fi

	annex_s=`echo ${annex_s:1}`
	annex_s=`echo ${annex_s%?}`

	# Evaluate Line Mode (according to G.997.1, 7.3.1.1.1)

	# Regional standard: ANSI T1.413
	if [ $((xtse1 & 1)) != 0  ]; then
		line_mode_s=" T1.413,"
	fi

	# Regional standard: TS 101 388
	if [ $((xtse1 & 1)) != 0  ]; then
		line_mode_s="$line_mode_s TS 101 388,"
	fi

	if [ $((xtse1 & 252)) != 0  ]; then
		line_mode_s="$line_mode_s G.992.1 (ADSL),"
	fi

	if [ $((xtse2 & 15)) != 0  ]; then
		line_mode_s="$line_mode_s G.992.2 (ADSL lite),"
	fi

	if [ $((xtse3 & 60)) != 0 \
	-o $((xtse4 & 240)) != 0 \
	-o $((xtse5 & 252)) != 0  ]; then
		line_mode_s="$line_mode_s G.992.3 (ADSL2),"
	fi

	if [ $((xtse4 & 3)) != 0 \
	-o $((xtse5 & 3)) != 0  ]; then
		line_mode_s="$line_mode_s G.992.4 (ADSL2 lite),"
	fi

	if [ $((xtse6 & 199)) != 0 \
	-o $((xtse7 & 15)) != 0  ]; then
		line_mode_s="$line_mode_s G.992.5 (ADSL2+),"
	fi

	if [ $((xtse8 & 7)) != 0  ]; then
		dsmsg=$(dsl_cmd dsmsg)
		vector_s=$(dsl_val "$dsmsg" eVectorStatus)

		case "$vector_s" in
			"0")	line_mode_s="$line_mode_s G.993.2 (VDSL2)," ;;
			"1")	line_mode_s="$line_mode_s G.993.5 (VDSL2 with downstream vectoring)," ;;
			"2")	line_mode_s="$line_mode_s G.993.5 (VDSL2 with down- and upstream vectoring)," ;;
			*)	line_mode_s="$line_mode_s unknown," ;;
		esac
	fi

	#!!! PROPRIETARY & INTERMEDIATE USE !!!
	if [ $((xtse8 & 128)) != 0  ]; then
		line_mode_s="$line_mode_s G.993.1 (VDSL),"
	fi

	line_mode_s=`echo ${line_mode_s:1}`
	line_mode_s=`echo ${line_mode_s%?}`

	xtse_s="${xtse1}, ${xtse2}, ${xtse3}, ${xtse4}, ${xtse5}, ${xtse6}, ${xtse7}, ${xtse8}"

	if [ "$action" = "lucistat" ]; then
		echo "dsl.xtse1=${xtse1:-nil}"
		echo "dsl.xtse2=${xtse2:-nil}"
		echo "dsl.xtse3=${xtse3:-nil}"
		echo "dsl.xtse4=${xtse4:-nil}"
		echo "dsl.xtse5=${xtse5:-nil}"
		echo "dsl.xtse6=${xtse6:-nil}"
		echo "dsl.xtse7=${xtse7:-nil}"
		echo "dsl.xtse8=${xtse8:-nil}"
		echo "dsl.xtse_s=\"$xtse_s\""
		echo "dsl.annex_s=\"${annex_s}\""
		echo "dsl.line_mode_s=\"${line_mode_s}\""
	else
		echo "XTSE Capabilities:                        ${xtse_s}"
		echo "Annex:                                    ${annex_s}"
		echo "Line Mode:                                ${line_mode_s}"
	fi
}

#
# Power Management Mode
#
power_mode() {
	local pmsg=$(dsl_cmd g997pmsg)
	local pm=$(dsl_val "$pmsg" nPowerManagementStatus);
	local s;

	case "$pm" in
		"-1")		s="Power management state is not available" ;;
		"0")		s="L0 - Synchronized" ;;
		"1")		s="L1 - Power Down Data transmission (G.992.2)" ;;
		"2")		s="L2 - Power Down Data transmission (G.992.3 and G.992.4)" ;;
		"3")		s="L3 - No power" ;;
		*)		s="unknown" ;;
	esac

	if [ "$action" = "lucistat" ]; then
		echo "dsl.power_mode_num=${pm:-nil}"
		echo "dsl.power_mode_s=\"$s\""
	else
		echo "Power Management Mode:                    $s"
	fi
}

#
# Latency type (interleave delay)
#
latency_delay() {
	local csg

	local idu
	local idu_s;
	local sidu

	local idd
	local idd_s;
	local sidd

	csg=$(dsl_cmd g997csg 0 1)
	idd=$(dsl_val "$csg" ActualInterleaveDelay)

	csg=$(dsl_cmd g997csg 0 0)
	idu=$(dsl_val "$csg" ActualInterleaveDelay)

	[ -z "$idd" ] && idd=0
	[ -z "$idu" ] && idu=0

	if [ "$idd" -gt 100 ]; then
		idd_s="Interleave"
	else
		idd_s="Fast"
	fi

	if [ "$idu" -gt 100 ]; then
		idu_s="Interleave"
	else
		idu_s="Fast"
	fi

	sidu=$(scale_latency $idu)
	sidd=$(scale_latency $idd)

	if [ "$action" = "lucistat" ]; then
		echo "dsl.latency_num_down=\"$sidd\""
		echo "dsl.latency_num_up=\"$sidu\""
		echo "dsl.latency_s_down=\"$idd_s\""
		echo "dsl.latency_s_up=\"$idu_s\""
	else
		echo "Latency [Interleave Delay]:               ${sidd} [${idd_s}]   ${sidu} [${idu_s}]"
	fi
}

#
# Errors
#
errors() {
	local lsctg
	local dpctg
	local ccsg
	local esf
	local esn
	local sesf
	local sesn
	local lossf
	local lossn
	local uasf
	local uasn

	local crc_pf
	local crc_pn
	local crcp_pf
	local crcp_pn
	local hecf
	local hecn

	local fecn
	local fecf

	lsctg=$(dsl_cmd pmlsctg 1)
	esf=$(dsl_val "$lsctg" nES)
	sesf=$(dsl_val "$lsctg" nSES)
	lossf=$(dsl_val "$lsctg" nLOSS)
	uasf=$(dsl_val "$lsctg" nUAS)

	lsctg=$(dsl_cmd pmlsctg 0)
	esn=$(dsl_val "$lsctg" nES)
	sesn=$(dsl_val "$lsctg" nSES)
	lossn=$(dsl_val "$lsctg" nLOSS)
	uasn=$(dsl_val "$lsctg" nUAS)

	dpctg=$(dsl_cmd pmdpctg 0 1)
	hecf=$(dsl_val "$dpctg" nHEC)
	crc_pf=$(dsl_val "$dpctg" nCRC_P)
	crcp_pf=$(dsl_val "$dpctg" nCRCP_P)

	dpctg=$(dsl_cmd pmdpctg 0 0)
	hecn=$(dsl_val "$dpctg" nHEC)
	crc_pn=$(dsl_val "$dpctg" nCRC_P)
	crcp_pn=$(dsl_val "$dpctg" nCRCP_P)

	ccsg=$(dsl_cmd pmccsg 0 1 0)
	fecf=$(dsl_val "$ccsg" nFEC)

	ccsg=$(dsl_cmd pmccsg 0 0 0)
	fecn=$(dsl_val "$ccsg" nFEC)

	if [ "$action" = "lucistat" ]; then
		echo "dsl.errors_fec_near=${fecn:-nil}"
		echo "dsl.errors_fec_far=${fecf:-nil}"
		echo "dsl.errors_es_near=${esn:-nil}"
		echo "dsl.errors_es_far=${esf:-nil}"
		echo "dsl.errors_ses_near=${sesn:-nil}"
		echo "dsl.errors_ses_far=${sesf:-nil}"
		echo "dsl.errors_loss_near=${lossn:-nil}"
		echo "dsl.errors_loss_far=${lossf:-nil}"
		echo "dsl.errors_uas_near=${uasn:-nil}"
		echo "dsl.errors_uas_far=${uasf:-nil}"
		echo "dsl.errors_hec_near=${hecn:-nil}"
		echo "dsl.errors_hec_far=${hecf:-nil}"
		echo "dsl.errors_crc_p_near=${crc_pn:-nil}"
		echo "dsl.errors_crc_p_far=${crc_pf:-nil}"
		echo "dsl.errors_crcp_p_near=${crcp_pn:-nil}"
		echo "dsl.errors_crcp_p_far=${crcp_pf:-nil}"
	else
		echo "Forward Error Correction Seconds (FECS):  Near: ${fecn} / Far: ${fecf}"
		echo "Errored seconds (ES):                     Near: ${esn} / Far: ${esf}"
		echo "Severely Errored Seconds (SES):           Near: ${sesn} / Far: ${sesf}"
		echo "Loss of Signal Seconds (LOSS):            Near: ${lossn} / Far: ${lossf}"
		echo "Unavailable Seconds (UAS):                Near: ${uasn} / Far: ${uasf}"
		echo "Header Error Code Errors (HEC):           Near: ${hecn} / Far: ${hecf}"
		echo "Non Pre-emtive CRC errors (CRC_P):        Near: ${crc_pn} / Far: ${crc_pf}"
		echo "Pre-emtive CRC errors (CRCP_P):           Near: ${crcp_pn} / Far: ${crcp_pf}"
	fi
}

#
# Work out how long the line has been up
#
line_uptime() {
	local ccsg
	local et
	local etr
	local d
	local h
	local m
	local s
	local rc=""

	ccsg=$(dsl_cmd pmccsg 0 0 0)
	et=$(dsl_val "$ccsg" nElapsedTime)

	[ -z "$et" ] && et=0

	d=$(expr $et / 86400)
	etr=$(expr $et % 86400)
	h=$(expr $etr / 3600)
	etr=$(expr $etr % 3600)
	m=$(expr $etr / 60)
	s=$(expr $etr % 60)


	[ "${d}${h}${m}${s}" -ne 0 ] && rc="${s}s"
	[ "${d}${h}${m}" -ne 0 ] && rc="${m}m ${rc}"
	[ "${d}${h}" -ne 0 ] && rc="${h}h ${rc}"
	[ "${d}" -ne 0 ] && rc="${d}d ${rc}"

	[ -z "$rc" ] && rc="down"


	if [ "$action" = "lucistat" ]; then
		echo "dsl.line_uptime=${et}"
		echo "dsl.line_uptime_s=\"${rc}\""
	else

		echo "Line Uptime Seconds:                      ${et}"
		echo "Line Uptime:                              ${rc}"
	fi
}

#
# Get noise and attenuation figures
#
line_data() {
	local lsg
	local latnu
	local latnd
	local satnu
	local satnd
	local snru
	local snrd
	local attndru
	local attndrd
	local sattndru
	local sattndrd
	local actatpu
	local actatpd

	lsg=$(dsl_cmd g997lsg 1 1)
	latnd=$(dsl_val "$lsg" LATN)
	satnd=$(dsl_val "$lsg" SATN)
	snrd=$(dsl_val "$lsg" SNR)
	attndrd=$(dsl_val "$lsg" ATTNDR)
	actatpd=$(dsl_val "$lsg" ACTATP)

	lsg=$(dsl_cmd g997lsg 0 1)
	latnu=$(dsl_val "$lsg" LATN)
	satnu=$(dsl_val "$lsg" SATN)
	snru=$(dsl_val "$lsg" SNR)
	attndru=$(dsl_val "$lsg" ATTNDR)
	actatpu=$(dsl_val "$lsg" ACTATP)

	[ -z "$latnd" ] && latnd=0
	[ -z "$latnu" ] && latnu=0
	[ -z "$satnd" ] && satnd=0
	[ -z "$satnu" ] && satnu=0
	[ -z "$snrd" ] && snrd=0
	[ -z "$snru" ] && snru=0
	[ -z "$actatpd" ] && actatpd=0
	[ -z "$actatpu" ] && actatpu=0

	latnd=$(dbt $latnd)
	latnu=$(dbt $latnu)
	satnd=$(dbt $satnd)
	satnu=$(dbt $satnu)
	snrd=$(dbt $snrd)
	snru=$(dbt $snru)
	actatpd=$(dbt $actatpd)
	actatpu=$(dbt $actatpu)

	[ -z "$attndrd" ] && attndrd=0
	[ -z "$attndru" ] && attndru=0

	sattndrd=$(scale $attndrd)
	sattndru=$(scale $attndru)
	
	if [ "$action" = "lucistat" ]; then
		echo "dsl.line_attenuation_down=\"$latnd\""
		echo "dsl.line_attenuation_up=\"$latnu\""
		echo "dsl.noise_margin_down=\"$snrd\""
		echo "dsl.noise_margin_up=\"$snru\""
		echo "dsl.signal_attenuation_down=\"$satnd\""
		echo "dsl.signal_attenuation_up=\"$satnu\""
		echo "dsl.actatp_down=\"$actatpd\""
		echo "dsl.actatp_up=\"$actatpu\""
		echo "dsl.max_data_rate_down=$attndrd"
		echo "dsl.max_data_rate_up=$attndru"
		echo "dsl.max_data_rate_down_s=\"$sattndrd\""
		echo "dsl.max_data_rate_up_s=\"$sattndru\""
	else
		echo "Line Attenuation (LATN):                  Down: ${latnd} dB / Up: ${latnu} dB"
		echo "Signal Attenuation (SATN):                Down: ${satnd} dB / Up: ${satnu} dB"
		echo "Noise Margin (SNR):                       Down: ${snrd} dB / Up: ${snru} dB"
		echo "Aggregate Transmit Power (ACTATP):        Down: ${actatpd} dB / Up: ${actatpu} dB"
		echo "Max. Attainable Data Rate (ATTNDR):       Down: ${sattndrd}/s / Up: ${sattndru}/s"
	fi
}

#
# Is the line up? Or what state is it in?
#
line_state() {
	local lsg=$(dsl_cmd lsg)
	local ls=$(dsl_val "$lsg" nLineState);
	local s;

	case "$ls" in
		"0x0")		s="not initialized" ;;
		"0x1")		s="exception" ;;
		"0x10")		s="not updated" ;;
		"0xff")		s="idle request" ;;
		"0x100")	s="idle" ;;
		"0x1ff")	s="silent request" ;;
		"0x200")	s="silent" ;;
		"0x300")	s="handshake" ;;
		"0x380")	s="full_init" ;;
		"0x400")	s="discovery" ;;
		"0x500")	s="training" ;;
		"0x600")	s="analysis" ;;
		"0x700")	s="exchange" ;;
		"0x800")	s="showtime_no_sync" ;;
		"0x801")	s="showtime_tc_sync" ;;
		"0x900")	s="fastretrain" ;;
		"0xa00")	s="lowpower_l2" ;;
		"0xb00")	s="loopdiagnostic active" ;;
		"0xb10")	s="loopdiagnostic data exchange" ;;
		"0xb20")	s="loopdiagnostic data request" ;;
		"0xc00")	s="loopdiagnostic complete" ;;
		"0x1000000")	s="test" ;;
		"0xd00")	s="resync" ;;
		"0x3c0")	s="short init entry" ;;
		"")		s="not running daemon"; ls="0xfff" ;;
		*)		s="unknown" ;;
	esac

	if [ "$action" = "lucistat" ]; then
		echo "dsl.line_state_num=$ls"
		echo "dsl.line_state_detail=\"$s\""
		if [ "$ls" = "0x801" ]; then
			echo "dsl.line_state=\"UP\""
		else
			echo "dsl.line_state=\"DOWN\""
		fi
	else
		if [ "$ls" = "0x801" ]; then
			echo "Line State:                               UP [$ls: $s]"
		else
			echo "Line State:                               DOWN [$ls: $s]"
		fi
	fi
}

#
# Which profile is used?
#
profile() {
	local bpstg=$(dsl_cmd bpstg)
	local profile=$(dsl_val "$bpstg" nProfile);
	local s;

	case "$profile" in
		"0")	s="8a" ;;
		"1")	s="8b" ;;
		"2")	s="8c" ;;
		"3")	s="8d" ;;
		"4")	s="12a" ;;
		"5")	s="12b" ;;
		"6")	s="17a" ;;
		"7")	s="30a" ;;
		"8")	s="17b" ;;
		"")		s="";;
		*)		s="unknown" ;;
	esac

	if [ "$action" = "lucistat" ]; then
		echo "dsl.profile=${profile:-nil}"
		echo "dsl.profile_s=\"${s}\""
	else
		echo "Profile:                                  $s"
	fi
}

status() {
	vendor
	chipset
	xtse
	profile
	line_state
	errors
	power_mode
	latency_delay
	data_rates
	line_data
	line_uptime
}

lucistat() {
	echo "local dsl={}"
	status
	echo "return dsl"
}
