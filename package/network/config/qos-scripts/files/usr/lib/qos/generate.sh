#!/bin/sh
[ -e /lib/functions.sh ] && . /lib/functions.sh || . ./functions.sh
[ -x /sbin/modprobe ] && {
	insmod="modprobe"
	rmmod="$insmod -r"
} || {
	insmod="insmod"
	rmmod="rmmod"
}

add_insmod() {
	eval "export isset=\${insmod_$1}"
	case "$isset" in
		1) ;;
		*) {
			[ "$2" ] && append INSMOD "$rmmod $1 >&- 2>&-" "$N"
			append INSMOD "$insmod $* >&- 2>&-" "$N"; export insmod_$1=1
		};;
	esac
}

[ -e /etc/config/network ] && {
	# only try to parse network config on openwrt

	find_ifname() {(
		reset_cb
		include /lib/network
		scan_interfaces
		config_get "$1" ifname
	)}
} || {
	find_ifname() {
		echo "Interface not found."
		exit 1
	}
}

parse_matching_rule() {
	local var="$1"
	local section="$2"
	local options="$3"
	local prefix="$4"
	local suffix="$5"
	local proto="$6"
	local mport=""
	local ports=""

	append "$var" "$prefix" "$N"
	for option in $options; do
		case "$option" in
			proto) config_get value "$section" proto; proto="${proto:-$value}";;
		esac
	done
	config_get type "$section" TYPE
	case "$type" in
		classify) unset pkt; append "$var" "-m mark --mark 0/0x0f";;
		default) pkt=1; append "$var" "-m mark --mark 0/0xf0";;
		reclassify) pkt=1;;
	esac
	append "$var" "${proto:+-p $proto}"
	for option in $options; do
		config_get value "$section" "$option"
		
		case "$pkt:$option" in
			*:srchost)
				append "$var" "-s $value"
			;;
			*:dsthost)
				append "$var" "-d $value"
			;;
			*:ports|*:srcports|*:dstports)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				lproto=${lproto:-tcp}
				case "$proto" in
					""|tcp|udp) append "$var" "-m ${proto:-tcp -p tcp} -m multiport";;
					*) unset "$var"; return 0;;
				esac
				case "$option" in
					ports)
						config_set "$section" srcports ""
						config_set "$section" dstports ""
						config_set "$section" portrange ""
						append "$var" "--ports $value"
					;;
					srcports)
						config_set "$section" ports ""
						config_set "$section" dstports ""
						config_set "$section" portrange ""
						append "$var" "--sports $value"
					;;
					dstports)
						config_set "$section" ports ""
						config_set "$section" srcports ""
						config_set "$section" portrange ""
						append "$var" "--dports $value"
					;;
				esac
				ports=1
			;;
			*:portrange)
				config_set "$section" ports ""
				config_set "$section" srcports ""
				config_set "$section" dstports ""
				value="$(echo "$value" | sed -e 's,-,:,g')"
				case "$proto" in
					""|tcp|udp) append "$var" "-m ${proto:-tcp -p tcp} --sport $value --dport $value";;
					*) unset "$var"; return 0;;
				esac
				ports=1
			;;
			*:connbytes)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				add_insmod xt_connbytes
				append "$var" "-m connbytes --connbytes $value --connbytes-dir both --connbytes-mode bytes"
			;;
			*:comment)
				add_insmod xt_comment
				append "$var" "-m comment --comment '$value'"
			;;
			*:tos)
				add_insmod xt_dscp
				case "$value" in
					!*) append "$var" "-m tos ! --tos $value";;
					*) append "$var" "-m tos --tos $value"
				esac
			;;
			*:dscp)
				add_insmod xt_dscp
				dscp_option="--dscp"
				[ -z "${value%%[EBCA]*}" ] && dscp_option="--dscp-class"
				case "$value" in
					!*) append "$var" "-m dscp ! $dscp_option $value";;
					*) append "$var" "-m dscp $dscp_option $value"
				esac
			;;
			*:direction)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				if [ "$value" = "out" ]; then
					append "$var" "-o $device"
				elif [ "$value" = "in" ]; then
					append "$var" "-i $device"
				fi
			;;
			*:srciface)
				append "$var" "-i $value"
			;;
			1:pktsize)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				add_insmod xt_length
				append "$var" "-m length --length $value"
			;;
			1:limit)
				add_insmod xt_limit
				append "$var" "-m limit --limit $value"
			;;
			1:tcpflags)
				case "$proto" in
					tcp) append "$var" "-m tcp --tcp-flags ALL $value";;
					*) unset $var; return 0;;
				esac
			;;
			1:mark)
				config_get class "${value##!}" classnr
				[ -z "$class" ] && continue;
				case "$value" in
					!*) append "$var" "-m mark ! --mark $class/0x0f";;
					*) append "$var" "-m mark --mark $class/0x0f";;
				esac
			;;
			1:TOS)
				add_insmod xt_DSCP
				config_get TOS "$rule" 'TOS'
				suffix="-j TOS --set-tos "${TOS:-"Normal-Service"}
			;;
			1:DSCP)
				add_insmod xt_DSCP
				config_get DSCP "$rule" 'DSCP'
				[ -z "${DSCP%%[EBCA]*}" ] && set_value="--set-dscp-class $DSCP" \
				|| set_value="--set-dscp $DSCP"
				suffix="-j DSCP $set_value"
			;;
		esac
	done
	append "$var" "$suffix"
	case "$ports:$proto" in
		1:)	parse_matching_rule "$var" "$section" "$options" "$prefix" "$suffix" "udp";;
	esac
}

config_cb() {
	option_cb() {
		return 0
	}
	case "$1" in
		interface)
			config_set "$2" "classgroup" "Default"
			config_set "$2" "upload" "128"
		;;
		classify|default|reclassify)
			option_cb() {
				append "CONFIG_${CONFIG_SECTION}_options" "$1"
			}
		;;
	esac
}

qos_parse_config() {
	config_get TYPE "$1" TYPE
	case "$TYPE" in
		interface)
			config_get_bool enabled "$1" enabled 1
			[ 1 -eq "$enabled" ] && {
				config_get classgroup "$1" classgroup
				config_set "$1" ifbdev "$C"
				C=$(($C+1))
				append INTERFACES "$1"
				config_set "$classgroup" enabled 1
				config_get device "$1" device
				[ -z "$device" ] && {
					device="$(find_ifname $1)"
					config_set "$1" device "$device"
				}
			}
		;;
		classgroup) append CG "$1";;
		classify|default|reclassify)
			case "$TYPE" in
				classify) var="ctrules";;
				*) var="rules";;
			esac
			append "$var" "$1"
		;;
	esac
}

enum_classes() {
	local c="0"
	config_get classes "$1" classes
	config_get default "$1" default
	for class in $classes; do
		c="$(($c + 1))"
		config_set "${class}" classnr $c
		case "$class" in
			$default) class_default=$c;;
		esac
	done
	class_default="${class_default:-$c}"
}

cls_var() {
	local varname="$1"
	local class="$2"
	local name="$3"
	local type="$4"
	local default="$5"
	local tmp tmp1 tmp2
	config_get tmp1 "$class" "$name"
	config_get tmp2 "${class}_${type}" "$name"
	tmp="${tmp2:-$tmp1}"
	tmp="${tmp:-$tmp2}"
	export ${varname}="${tmp:-$default}"
}

tcrules() {
	_dir=/usr/lib/qos
	[ -e $_dir/tcrules.awk ] || _dir=.
	echo "$cstr" | awk \
		-v device="$dev" \
		-v linespeed="$rate" \
		-v direction="$dir" \
		-f $_dir/tcrules.awk
}

start_interface() {
	local iface="$1"
	local num_ifb="$2"
	config_get device "$iface" device
	config_get_bool enabled "$iface" enabled 1
	[ -z "$device" -o 1 -ne "$enabled" ] && {
		return 1 
	}
	config_get upload "$iface" upload
	config_get_bool halfduplex "$iface" halfduplex
	config_get download "$iface" download
	config_get classgroup "$iface" classgroup
	config_get_bool overhead "$iface" overhead 0
	
	download="${download:-${halfduplex:+$upload}}"
	enum_classes "$classgroup"
	for dir in ${halfduplex:-up} ${download:+down}; do
		case "$dir" in
			up)
				[ "$overhead" = 1 ] && upload=$(($upload * 98 / 100 - (15 * 128 / $upload)))
				dev="$device"
				rate="$upload"
				dl_mode=""
				prefix="cls"
			;;
			down)
				[ "$(ls -d /proc/sys/net/ipv4/conf/ifb* 2>&- | wc -l)" -ne "$num_ifb" ] && add_insmod ifb numifbs="$num_ifb"
				config_get ifbdev "$iface" ifbdev
				[ "$overhead" = 1 ] && download=$(($download * 98 / 100 - (80 * 1024 / $download)))
				dev="ifb$ifbdev"
				rate="$download"
				dl_mode=1
				prefix="d_cls"
			;;
			*) continue;;
		esac
		cstr=
		for class in $classes; do
			cls_var pktsize "$class" packetsize $dir 1500
			cls_var pktdelay "$class" packetdelay $dir 0
			cls_var maxrate "$class" limitrate $dir 100
			cls_var prio "$class" priority $dir 1
			cls_var avgrate "$class" avgrate $dir 0
			cls_var qdisc "$class" qdisc $dir ""
			cls_var filter "$class" filter $dir ""
			config_get classnr "$class" classnr
			append cstr "$classnr:$prio:$avgrate:$pktsize:$pktdelay:$maxrate:$qdisc:$filter" "$N"
		done
		append ${prefix}q "$(tcrules)" "$N"
		export dev_${dir}="ifconfig $dev up >&- 2>&-
tc qdisc del dev $dev root >&- 2>&-
tc qdisc add dev $dev root handle 1: hfsc default ${class_default}0
tc class add dev $dev parent 1: classid 1:1 hfsc sc rate ${rate}kbit ul rate ${rate}kbit"
	done
	[ -n "$download" ] && {
		add_insmod cls_u32
		add_insmod em_u32
		add_insmod act_connmark
		add_insmod act_mirred
		add_insmod sch_ingress
	}
	if [ -n "$halfduplex" ]; then
		export dev_up="tc qdisc del dev $device root >&- 2>&-
tc qdisc add dev $device root handle 1: hfsc
tc filter add dev $device parent 1: prio 10 u32 match u32 0 0 flowid 1:1 action mirred egress redirect dev ifb$ifbdev"
	elif [ -n "$download" ]; then
		append dev_${dir} "tc qdisc del dev $device ingress >&- 2>&-
tc qdisc add dev $device ingress
tc filter add dev $device parent ffff: prio 1 u32 match u32 0 0 flowid 1:1 action connmark action mirred egress redirect dev ifb$ifbdev" "$N"
	fi
	add_insmod cls_fw
	add_insmod sch_hfsc

	cat <<EOF
${INSMOD:+$INSMOD$N}${dev_up:+$dev_up
$clsq
}${ifbdev:+$dev_down
$d_clsq
$d_clsl
$d_clsf
}
EOF
	unset INSMOD clsq clsf clsl d_clsq d_clsl d_clsf dev_up dev_down
}

start_interfaces() {
	local C="$1"
	for iface in $INTERFACES; do
		start_interface "$iface" "$C"
	done
}

add_rules() {
	local var="$1"
	local rules="$2"
	local prefix="$3"
	
	for rule in $rules; do
		unset iptrule
		config_get target "$rule" target
		config_get target "$target" classnr
		config_get options "$rule" options

		## If we want to override the TOS field, let's clear the DSCP field first.
		[ ! -z "$(echo $options | grep 'TOS')" ] && {
			s_options=${options%%TOS}
			add_insmod xt_DSCP
			parse_matching_rule iptrule "$rule" "$s_options" "$prefix" "-j DSCP --set-dscp 0"
			append "$var" "$iptrule" "$N"
			unset iptrule
		}

		target=$(($target | ($target << 4)))
		parse_matching_rule iptrule "$rule" "$options" "$prefix" "-j MARK --set-mark $target/0xff"
		append "$var" "$iptrule" "$N"
	done
}

start_cg() {
	local cg="$1"
	local iptrules
	local pktrules
	local sizerules
	enum_classes "$cg"
	for command in $iptables; do
		add_rules iptrules "$ctrules" "$command -w -t mangle -A qos_${cg}_ct"
	done
	config_get classes "$cg" classes
	for class in $classes; do
		config_get mark "$class" classnr
		config_get maxsize "$class" maxsize
		[ -z "$maxsize" -o -z "$mark" ] || {
			add_insmod xt_length
			for command in $iptables; do
				append pktrules "$command -w -t mangle -A qos_${cg} -m mark --mark $mark/0x0f -m length --length $maxsize: -j MARK --set-mark 0/0xff" "$N"
			done
		}
	done
	for command in $iptables; do
		add_rules pktrules "$rules" "$command -w -t mangle -A qos_${cg}"
	done
	for iface in $INTERFACES; do
		config_get classgroup "$iface" classgroup
		config_get device "$iface" device
		config_get ifbdev "$iface" ifbdev
		config_get upload "$iface" upload
		config_get download "$iface" download
		config_get halfduplex "$iface" halfduplex
		download="${download:-${halfduplex:+$upload}}"
		for command in $iptables; do
			append up "$command -w -t mangle -A OUTPUT -o $device -j qos_${cg}" "$N"
			append up "$command -w -t mangle -A FORWARD -o $device -j qos_${cg}" "$N"
		done
	done
	cat <<EOF
$INSMOD
EOF

for command in $iptables; do
	cat <<EOF
	$command -w -t mangle -N qos_${cg} 
	$command -w -t mangle -N qos_${cg}_ct
EOF
done
cat <<EOF
	${iptrules:+${iptrules}${N}}
EOF
for command in $iptables; do
	cat <<EOF
	$command -w -t mangle -A qos_${cg}_ct -j CONNMARK --save-mark --mask 0xff
	$command -w -t mangle -A qos_${cg} -j CONNMARK --restore-mark --mask 0x0f
	$command -w -t mangle -A qos_${cg} -m mark --mark 0/0x0f -j qos_${cg}_ct
EOF
done
cat <<EOF
$pktrules
EOF
for command in $iptables; do
	cat <<EOF
	$command -w -t mangle -A qos_${cg} -j CONNMARK --save-mark --mask 0xff
EOF
done
cat <<EOF
$up$N${down:+${down}$N}
EOF
	unset INSMOD
}

start_firewall() {
	add_insmod xt_multiport
	add_insmod xt_connmark
	stop_firewall
	for group in $CG; do
		start_cg $group
	done
}

stop_firewall() {
	# Builds up a list of iptables commands to flush the qos_* chains,
	# remove rules referring to them, then delete them

	# Print rules in the mangle table, like iptables-save
	for command in $iptables; do
		$command -w -t mangle -S |
			# Find rules for the qos_* chains
			grep -E '(^-N qos_|-j qos_)' |
			# Exclude rules in qos_* chains (inter-qos_* refs)
			grep -v '^-A qos_' |
			# Replace -N with -X and hold, with -F and print
			# Replace -A with -D
			# Print held lines at the end (note leading newline)
			sed -e '/^-N/{s/^-N/-X/;H;s/^-X/-F/}' \
				-e 's/^-A/-D/' \
				-e '${p;g}' |
			# Make into proper iptables calls
			# Note: awkward in previous call due to hold space usage
			sed -n -e "s/^./${command} -w -t mangle &/p"
	done
}

C="0"
INTERFACES=""
[ -e ./qos.conf ] && {
	. ./qos.conf
	config_cb
} || {
	config_load qos
	config_foreach qos_parse_config
}

C="0"
for iface in $INTERFACES; do
	export C="$(($C + 1))"
done

[ -x /usr/sbin/ip6tables ] && {
	iptables="ip6tables iptables"
} || {
	iptables="iptables"
}

case "$1" in
	all)
		start_interfaces "$C"
		start_firewall
	;;
	interface)
		start_interface "$2" "$C"
	;;
	interfaces)
		start_interfaces
	;;
	firewall)
		case "$2" in
			stop)
				stop_firewall
			;;
			start|"")
				start_firewall
			;;
		esac
	;;
esac
