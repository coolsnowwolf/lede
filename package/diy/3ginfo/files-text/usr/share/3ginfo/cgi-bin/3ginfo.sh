#!/bin/sh

#
# (c) 2010-2019 Cezary Jackiewicz <cezary@eko.one.pl>
#

# Output format
# 0 - html
# 1 - txt
# 2 - json

FORMAT=1
RES="/usr/share/3ginfo"

LANG=$(uci -q get 3ginfo.@3ginfo[0].language)
[ "x$LANG" = "x" ] && LANG="en"

getpath() {
	devname="$(basename "$1")"
	case "$devname" in
	'tty'*)
		devpath="$(readlink -f /sys/class/tty/$devname/device)"
		P=${devpath%/*/*}
		;;
	*)
		devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
		P=${devpath%/*}
		;;
	esac
}

showjsonerror() {
	echo "{\"error\": \"$1\"}"
}

if [ "x$1" = "xjson" ]; then
	FORMAT=2
else
	if [ "`basename $0`" = "3ginfo" ]; then
		FORMAT=1
	else
		FORMAT=0
		echo -e "Content-type: text/html\n\n"
	fi

	if [ ! -e $RES/msg.dat.$LANG ]; then
		echo "File missing: $RES/msg.dat.$LANG"
		exit 0
	fi
	. $RES/msg.dat.$LANG
fi

# odpytanie urzadzenia
DEVICE=$(uci -q get 3ginfo.@3ginfo[0].device)

if echo "x$DEVICE" | grep -q "192.168."; then
	if grep -q "Vendor=1bbb" /sys/kernel/debug/usb/devices; then
		O=$($RES/scripts/alcatel_hilink.sh $DEVICE)
	fi
	if grep -q "Vendor=12d1" /sys/kernel/debug/usb/devices; then
		O=$($RES/scripts/huawei_hilink.sh $DEVICE)
	fi
	if grep -q "Vendor=19d2" /sys/kernel/debug/usb/devices; then
		O=$($RES/scripts/zte.sh $DEVICE)
	fi
	SEC=$(uci -q get 3ginfo.@3ginfo[0].network)
	SEC=${SEC:-wan}
else
	if [ "x$DEVICE" = "x" ]; then
		devices=$(ls /dev/ttyUSB* /dev/cdc-wdm* /dev/ttyACM* /dev/ttyHS* 2>/dev/null | sort -r);
		for d in $devices; do
			DEVICE=$d gcom -s $RES/scripts/probeport.gcom > /dev/null 2>&1
			if [ $? = 0 ]; then
				uci set 3ginfo.@3ginfo[0].device="$d"
				uci commit 3ginfo
				break
			fi
		done
		DEVICE=$(uci -q get 3ginfo.@3ginfo[0].device)
	fi

	if [ "x$DEVICE" = "x" ]; then
		[ $FORMAT -eq 0 ] && echo "<h3 style='color:red;' class=\"c\">$NOTDETECTED</h3>"
		[ $FORMAT -eq 1 ] && echo $NOTDETECTED
		[ $FORMAT -eq 2 ] && showjsonerror "NOTDETECTED"
		exit 0
	fi

	if [ ! -e $DEVICE ]; then
		[ $FORMAT -eq 0 ] && echo "<h3 style='color:red;' class=\"c\">$NODEVICE $DEVICE!</h3>"
		[ $FORMAT -eq 1 ] && echo "$NODEVICE $DEVICE."
		[ $FORMAT -eq 2 ] && showjsonerror "NODEVICE $DEVICE"
		exit 0
	fi

	# znajdz odpowiednia sekcje w konfiguracji
	SEC=$(uci -q get 3ginfo.@3ginfo[0].network)
	if [ -z "$SEC" ]; then
		getpath $DEVICE
		PORIG=$P
		for DEV in /sys/class/tty/* /sys/class/usbmisc/*; do
			getpath "/dev/"${DEV##/*/}
			if [ "x$PORIG" = "x$P" ]; then
				SEC=$(uci show network | grep "/dev/"${DEV##/*/} | cut -f2 -d.)
				[ -n "$SEC" ] && break
			fi
		done
	fi

#	[ "${DEVICE%%[0-9]}" = "/dev/ttyUSB" ] && stty -F $DEVICE -iexten -opost -icrnl

	# daj pin jak jest taka potrzeba
	if [ ! -f /tmp/pincode_was_given ]; then
		# tylko za pierwszym razem
		if [ ! -z $SEC ]; then
			PINCODE=$(uci -q get network.$SEC.pincode)
		fi
		if [ -z "$PINCODE" ]; then
			PINCODE=$(uci -q get 3ginfo.@3ginfo[0].pincode)
		fi
		if [ ! -z $PINCODE ]; then
			PINCODE="$PINCODE" gcom -d "$DEVICE" -s /etc/gcom/setpin.gcom > /dev/null || {
				[ $FORMAT -eq 0 ] && echo "<h3 style='color:red;' class=\"c\">$PINERROR</h3>"
				[ $FORMAT -eq 1 ] && echo "$PINERROR"
				[ $FORMAT -eq 2 ] && showjsonerror "PINERROR"
				exit 0
			}
		fi
		touch /tmp/pincode_was_given
	fi

	O=$(gcom -d $DEVICE -s $RES/scripts/3ginfo.gcom 2>/dev/null)
fi

if [ "x$1" = "xtest" ]; then
	echo "$O"
	echo "---------------------------------------------------------------"
	ls /dev/tty*
	echo "---------------------------------------------------------------"
	cat /sys/kernel/debug/usb/devices
	echo "---------------------------------------------------------------"
	uci show 3ginfo
	exit 0
fi

# CSQ
CSQ=$(echo "$O" | awk -F[,\ ] '/^\+CSQ/ {print $2}')

[ "x$CSQ" = "x" ] && CSQ=-1
if [ $CSQ -ge 0 -a $CSQ -le 31 ]; then

	# for Gargoyle
	[ -e /tmp/strength.txt ] && echo "+CSQ: $CSQ,99" > /tmp/strength.txt

	CSQ_PER=$(($CSQ * 100/31))
	CSQ_COL="red"
	[ $CSQ -ge 10 ] && CSQ_COL="orange"
	[ $CSQ -ge 15 ] && CSQ_COL="yellow"
	[ $CSQ -ge 20 ] && CSQ_COL="green"
	CSQ_RSSI=$((2 * CSQ - 113))
else
	CSQ="-"
	CSQ_PER="0"
	CSQ_COL="black"
	CSQ_RSSI="-"
fi

# COPS
COPS_NUM=$(echo "$O" | awk -F[\"] '/^\+COPS: .,2/ {print $2}')
if [ "x$COPS_NUM" = "x" ]; then
	COPS_NUM="-"
	COPS_MCC="-"
	COPS_MNC="-"
else
	COPS_MCC=${COPS_NUM:0:3}
	COPS_MNC=${COPS_NUM:3:3}
	COPS=$(awk -F[\;] '/'$COPS_NUM'/ {print $2}' $RES/mccmnc.dat)
	[ "x$COPS" = "x" ] && COPS="-"
fi

# dla modemow Option i ZTE
if [ "$COPS_NUM" = "-" ]; then
	COPS=$(echo "$O" | awk -F[\"] '/^\+COPS: .,0/ {print $2}')
	[ "x$COPS" = "x" ] && COPS="---"

	COPS_TMP=$(awk -F[\;] 'BEGIN {IGNORECASE = 1} /'"$COPS"'/ {print $2}' $RES/mccmnc.dat)
	if [ "x$COPS_TMP" = "x" ]; then
		COPS_NUM="-"
		COPS_MCC="-"
		COPS_MNC="-"
	else
		COPS="$COPS_TMP"
		COPS_NUM=$(awk -F[\;] 'BEGIN {IGNORECASE = 1} /'"$COPS"'/ {print $1}' $RES/mccmnc.dat)
		COPS_MCC=${COPS_NUM:0:3}
		COPS_MNC=${COPS_NUM:3:3}
	fi
fi

# Technologia
MODE="-"

# Nowe Huawei
TECH=$(echo "$O" | awk -F[,] '/^\^SYSINFOEX/ {print $9}' | sed 's/"//g')
if [ "x$TECH" != "x" ]; then
	MODE=$(echo "$TECH" | sed 's/-//g')
fi

# Starsze modele Huawei i inne pozostale
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,] '/^\^SYSINFO/ {print $7}')
	case $TECH in
		17*) MODE="HSPA+ (64QAM)";;
		18*) MODE="HSPA+ (MIMO)";;
		1*) MODE="GSM";;
		2*) MODE="GPRS";;
		3*) MODE="EDGE";;
		4*) MODE="UMTS";;
		5*) MODE="HSDPA";;
		6*) MODE="HSUPA";;
		7*) MODE="HSPA";;
		9*) MODE="HSPA+";;
		 *) MODE="-";;
	esac
fi

# ZTE
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,\ ] '/^\+ZPAS/ {print $2}' | sed 's/"//g')
	if [ "x$TECH" != "x" -a "x$TECH" != "xNo" ]; then
		MODE="$TECH"
	fi
fi

# OPTION
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F, '/^\+COPS: 0/ {print $4}')
	MODE="-"
	if [ "$TECH" = 0 ]; then
		TECH1=$(echo "$O" | awk '/^_OCTI/ {print $2}' | cut -f1 -d,)
		case $TECH1 in
			1*) MODE="GSM";;
			2*) MODE="GPRS";;
			3*) MODE="EDGE";;
			 *) MODE="-";;
		esac
	elif [ "$TECH" = 2 ]; then
		TECH1=$(echo "$O" | awk '/^_OWCTI/ {print $2}')
		case $TECH1 in
			1*) MODE="UMTS";;
			2*) MODE="HSDPA";;
			3*) MODE="HSUPA";;
			4*) MODE="HSPA";;
			 *) MODE="-";;
		esac
	fi
fi

# Sierra
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,\ ] '/^\*CNTI/ {print $3}' | sed 's|/|,|g')
	if [ "x$TECH" != "x" ]; then
		MODE="$TECH"
	fi
fi

# Novatel
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,\ ] '/^\$CNTI/ {print $4}' | sed 's|/|,|g')
	if [ "x$TECH" != "x" ]; then
		MODE="$TECH"
	fi
fi

# Vodafone - icera
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,\ ] '/^\%NWSTATE/ {print $4}' | sed 's|/|,|g')
	if [ "x$TECH" != "x" ]; then
		MODE="$TECH"
	fi
fi

# SIMCOM
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,\ ] '/^\+CNSMOD/ {print $3}')
	case "$TECH" in
		1*) MODE="GSM";;
		2*) MODE="GPRS";;
		3*) MODE="EDGE";;
		4*) MODE="UMTS";;
		5*) MODE="HSDPA";;
		6*) MODE="HSUPA";;
		7*) MODE="HSPA";;
		 *) MODE="-";;
	esac
fi

# generic 3GPP TS 27.007 V10.4.0
if [ "x$MODE" = "x-" ]; then
	TECH=$(echo "$O" | awk -F[,] '/^\+COPS/ {print $4}')
	case "$TECH" in
		2*) MODE="UMTS";;
		3*) MODE="EDGE";;
		4*) MODE="HSDPA";;
		5*) MODE="HSUPA";;
		6*) MODE="HSPA";;
		7*) MODE="LTE";;
		 *) MODE="-";;
	esac
fi

# CREG
CREG="+CGREG"
LAC=$(echo "$O" | awk -F[,] '/\'$CREG'/ {printf "%s", toupper($3)}' | sed 's/[^A-F0-9]//g')
if [ "x$LAC" = "x" ]; then
	CREG="+CREG"
	LAC=$(echo "$O" | awk -F[,] '/\'$CREG'/ {printf "%s", toupper($3)}' | sed 's/[^A-F0-9]//g')
fi

if [ "x$LAC" != "x" ]; then
	LAC_NUM=$(printf %d 0x$LAC)
else
	LAC="-"
	LAC_NUM="-"
fi

# TAC
TAC=$(echo "$O" | awk -F[,] '/^\+CEREG/ {printf "%s", toupper($3)}' | sed 's/[^A-F0-9]//g')
if [ "x$TAC" != "x" ]; then
	TAC_NUM=$(printf %d 0x$TAC)
else
	TAC="-"
	TAC_NUM="-"
fi


# ECIO / RSCP
ECIO="-"
RSCP="-"

ECIx=$(echo "$O" | awk -F[,\ ] '/^\+ZRSSI:/ {print $3}')
if [ "x$ECIx" != "x" ]; then
	ECIO=`expr $ECIx / 2`
	ECIO="-"$ECIO
fi

RSCx=$(echo "$O" | awk -F[,\ ] '/^\+ZRSSI:/ {print $4}')
	if [ "x$RSCx" != "x" ]; then
		RSCP=`expr $RSCx / 2`
		RSCP="-"$RSCP
fi

RSCx=$(echo "$O" | awk -F[,\ ] '/^\^CSNR:/ {print $2}')
if [ "x$RSCx" != "x" ]; then
	RSCP=$RSCx
fi

ECIx=$(echo "$O" | awk -F[,\ ] '/^\^CSNR:/ {print $3}')
if [ "x$ECIx" != "x" ]; then
	ECIO=$ECIx
fi

# RSRP / RSRQ
RSRP="-"
RSRQ="-"
SINR="-"
RSRx=$(echo "$O" | awk -F[,:] '/^\^LTERSRP:/ {print $2}')
if [ "x$RSRx" != "x" ]; then
	RSRP=$RSRx
	RSRQ=$(echo "$O" | awk -F[,:] '/^\^LTERSRP:/ {print $3}')
fi

TECH=$(echo "$O" | awk -F[,:] '/^\^HCSQ:/ {print $2}' | sed 's/[" ]//g')
if [ "x$TECH" != "x" ]; then
	PARAM2=$(echo "$O" | awk -F[,:] '/^\^HCSQ:/ {print $4}')
	PARAM3=$(echo "$O" | awk -F[,:] '/^\^HCSQ:/ {print $5}')
	PARAM4=$(echo "$O" | awk -F[,:] '/^\^HCSQ:/ {print $6}')

	case "$TECH" in
		WCDMA*)
			RSCP=$(awk 'BEGIN {print -121 + '$PARAM2'}')
			ECIO=$(awk 'BEGIN {print -32.5 + '$PARAM3'/2}')
			;;
		LTE*)
			RSRP=$(awk 'BEGIN {print -141 + '$PARAM2'}')
			SINR=$(awk 'BEGIN {print -20.2 + '$PARAM3'/5}')
			RSRQ=$(awk 'BEGIN {print -20 + '$PARAM4'/2}')
			;;
	esac
fi

if [ -n "$SEC" ]; then
	if [ "x$(uci -q get network.$SEC.proto)" = "xqmi" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_load "$(uqmi -d "$(uci -q get network.$SEC.device)" --get-signal-info)" >/dev/null 2>&1
		json_get_var T type
		if [ "x$T" = "xlte" ]; then
			json_get_var RSRP rsrp
			json_get_var RSRQ rsrq
		fi
		if [ "x$T" = "xwcdma" ]; then
			json_get_var ECIO ecio
			json_get_var RSSI rssi
			json_get_var RSCP rscp
			if [ -z "$RSCP" ]; then
				RSCP=$((RSSI+ECIO))
			fi
		fi
	fi
fi

BTSINFO=""
CID=$(echo "$O" | awk -F[,] '/\'$CREG'/ {printf "%s", toupper($4)}' | sed 's/[^A-F0-9]//g')
if [ "x$CID" != "x" ]; then
	CID_NUM=$(printf %d 0x$CID)

	if [ ${#CID} -gt 4 ]; then
		T=$(echo "$CID" | awk '{print substr($1,length(substr($1,1,length($1)-4))+1)}')
	else
		T=$CID
	fi

	CLF=$(uci -q get 3ginfo.@3ginfo[0].clf)
	if [ -e "$CLF" ]; then
		PAT="xxx"
		[ "x$T" != "x-" -a "x$LAC_NUM" != "x-" ] && PAT="^$COPS_NUM;0x"$(printf %04X 0x$T)";0x"$(printf %04X $LAC_NUM)";"
		is_gz=$(dd if="$CLF" bs=1 count=2 2>/dev/null | hexdump -v -e '1/1 "%02x"')
		if [ "x$is_gz" = "x1f8b" ] ; then
			BTSINFO="<a href=\"http://maps.google.pl/?t=k\&z=17\&q="$(zcat "$CLF" | awk -F";" '/'$PAT'/ {printf $5","$6}')"\">"$(zcat "$CLF" | awk -F";" '/'$PAT'/ {gsub(/\!/,"\\!");print $8}')"</a>"
		else
			BTSINFO="<a href=\"http://maps.google.pl/?t=k\&z=17\&q="$(awk -F";" '/'$PAT'/ {printf $5","$6}' "$CLF")"\">"$(awk -F";" '/'$PAT'/ {gsub(/\!/,"\\!");print $8}' "$CLF")"</a>"
		fi
		if [ $FORMAT -eq 2 ]; then
			BTSINFO=$(echo "$BTSINFO" | sed 's!<a.*>\(.*\)</a>!\1!g')
		fi
	fi

	if [ $FORMAT -eq 0 ]; then
		case $COPS_NUM in
			26001*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=3\&amp;mode=adv\">$CID</a>";;
			26002*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=-1\&amp;mode=adv\">$CID</a>";;
			26003*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=-1\&amp;mode=adv\">$CID</a>";;
			26006*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=4\&amp;mode=adv\">$CID</a>";;
			26016*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=7\&amp;mode=adv\">$CID</a>";;
			26017*) CID="<a href=\"http://btsearch.pl/szukaj.php?search="$CID"h\&amp;siec=8\&amp;mode=adv\">$CID</a>";;
		esac
	fi
else
	CID="-"
	CID_NUM="-"
fi

# CGEQNEG
QOS_SHOW="none"
DOWN="-"
UP="-"

DOWx=$(echo "$O" | awk -F[,] '/\+CGEQNEG/ {printf "%s", $4}')
if [ "x$DOWx" != "x" ]; then
	DOWN=$DOWx
	UP=$(echo "$O" | awk -F[,] '/\+CGEQNEG/ {printf "%s", $3}')
	QOS_SHOW="block"
fi

# SMS
if [ -e /usr/bin/gnokii ]; then
	SMS_SHOW="block"
else
	SMS_SHOW="none"
fi

# USSD
which ussd159 >/dev/null 2>&1
if [ $? -eq 0 ]; then
	USSD_SHOW="block"
else
	USSD_SHOW="none"
fi

# Stan limitu
LIMIT_SHOW="none"
LIMIT=$(uci -q get 3ginfo.@3ginfo[0].script)
if [ "x$LIMIT" != "x" ]; then
	LIMIT_SHOW="block"
fi

# Status polaczenia

CONN_TIME="-"
RX="-"
TX="-"

if [ -z "$SEC" ]; then
	STATUS=$NOINFO
	[ $FORMAT -eq 2 ] && STATUS="NOINFO"

	STATUS_TRE="-"
	STATUS_SHOW="none"
	STATUS_SHOW_BUTTON="none"
else
	NETUP=$(ifstatus $SEC | grep "\"up\": true")
	if [ -n "$NETUP" ]; then
		[ $FORMAT -eq 0 ] && STATUS="<font color=green>$CONNECTED</font>"
		[ $FORMAT -eq 1 ] && STATUS=$CONNECTED
		[ $FORMAT -eq 2 ] && STATUS="CONNECTED"
		STATUS_TRE=$DISCONNECT

		CT=$(uci -q -P /var/state/ get network.$SEC.connect_time)
		if [ -z $CT ]; then
			CT=$(ifstatus $SEC | awk -F[:,] '/uptime/ {print $2}' | xargs)
		else
			UPTIME=$(cut -d. -f1 /proc/uptime)
			CT=$((UPTIME-CT))
		fi
		if [ ! -z $CT ]; then
			D=$(expr $CT / 60 / 60 / 24)
			H=$(expr $CT / 60 / 60 % 24)
			M=$(expr $CT / 60 % 60)
			S=$(expr $CT % 60)
			CONN_TIME=$(printf "%dd, %02d:%02d:%02d" $D $H $M $S)
		fi
		IFACE=$(ifstatus $SEC | awk -F\" '/l3_device/ {print $4}')
		if [ -n "$IFACE" ]; then
			RX=$(ifconfig $IFACE | awk -F[\(\)] '/bytes/ {printf "%s",$2}')
			TX=$(ifconfig $IFACE | awk -F[\(\)] '/bytes/ {printf "%s",$4}')
		fi
	else

		[ $FORMAT -eq 0 ] && STATUS="<font color=red>$DISCONNECTED</font>"
		[ $FORMAT -eq 1 ] && STATUS=$DISCONNECTED
		[ $FORMAT -eq 2 ] && STATUS="DISCONNECTED"
		STATUS_TRE=$CONNECT
	fi
	STATUS_SHOW="block"
	STATUS_SHOW_BUTTON="block"
fi

if [ "x"$(uci -q get 3ginfo.@3ginfo[0].connect_button) = "x0" ]; then
	STATUS_SHOW_BUTTON="none"
fi

# Informacja o urzadzeniu
DEVICE=$(echo "$O" | awk -F[:] '/DEVICE/ { print $2}')
if [ "x$DEVICE" = "x" ]; then
	DEVICE="-"
fi

# podmiana w szablonie
if [ $FORMAT -eq 2 ]; then
	TEMPLATE="$RES/status.json"
else
	[ $FORMAT -eq 0 ] && EXT="html"
	[ $FORMAT -eq 1 ] && EXT="txt"
	TEMPLATE="$RES/status.$EXT.$LANG"
fi

if [ -e $TEMPLATE ]; then
	sed -e "s!{CSQ}!$CSQ!g; \
	s!{CSQ_PER}!$CSQ_PER!g; \
	s!{CSQ_RSSI}!$CSQ_RSSI!g; \
	s!{CSQ_COL}!$CSQ_COL!g; \
	s!{COPS}!$COPS!g; \
	s!{COPS_NUM}!$COPS_NUM!g; \
	s!{COPS_MCC}!$COPS_MCC!g; \
	s!{COPS_MNC}!$COPS_MNC!g; \
	s!{LAC}!$LAC!g; \
	s!{LAC_NUM}!$LAC_NUM!g; \
	s!{CID}!$CID!g; \
	s!{CID_NUM}!$CID_NUM!g; \
	s!{TAC}!$TAC!g; \
	s!{TAC_NUM}!$TAC_NUM!g; \
	s!{BTSINFO}!$BTSINFO!g; \
	s!{DOWN}!$DOWN!g; \
	s!{UP}!$UP!g; \
	s!{QOS_SHOW}!$QOS_SHOW!g; \
	s!{SMS_SHOW}!$SMS_SHOW!g; \
	s!{USSD_SHOW}!$USSD_SHOW!g; \
	s!{LIMIT_SHOW}!$LIMIT_SHOW!g; \
	s!{STATUS}!$STATUS!g; \
	s!{CONN_TIME}!$CONN_TIME!g; \
	s!{CONN_TIME_SEC}!$CT!g; \
	s!{RX}!$RX!g; \
	s!{TX}!$TX!g; \
	s!{STATUS_TRE}!$STATUS_TRE!g; \
	s!{STATUS_SHOW}!$STATUS_SHOW!g; \
	s!{STATUS_SHOW_BUTTON}!$STATUS_SHOW_BUTTON!g; \
	s!{DEVICE}!$DEVICE!g; \
	s!{ECIO}!$ECIO!g; \
	s!{RSCP}!$RSCP!g; \
	s!{RSRP}!$RSRP!g; \
	s!{RSRQ}!$RSRQ!g; \
	s!{SINR}!$SINR!g; \
	s!{MODE}!$MODE!g" $TEMPLATE
else
	echo "Template $TEMPLATE missing!"
fi

exit 0
