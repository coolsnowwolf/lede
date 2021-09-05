#!/bin/sh

. /usr/share/libubox/jshn.sh

HOSTNAME="${COLLECTD_HOSTNAME:-localhost}"
INTERVAL="${COLLECTD_INTERVAL:-60}"

handle_cake() {
	local ifc ifr tin i

	ifc="$1"
	ifr="${ifc//[!0-9A-Za-z]/_}"

	# Overall
	json_get_vars bytes packets drops backlog qlen

	# Options
	json_select options
	json_get_vars bandwidth diffserv
	json_select ".."

	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_bytes\" interval=$INTERVAL N:$bytes"
	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_drops\" interval=$INTERVAL N:$drops"
	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_backlog\" interval=$INTERVAL N:$backlog"

	# ash doesn't have arrays so prepare to get a little creative
	case "$diffserv" in
		diffserv3 | diffserv4) tns="BKBEVIVO"
			;;
		*) tns="T0T1T2T3T4T5T6T7"
			;;
	esac

	# Tins
	# Flows & delays indicate the state as of the last packet that flowed through, so they appear to get stuck.
	# Discard the results from a stuck tin.
	json_get_keys tins tins
	json_select tins
	i=0
	for tin in $tins; do
		json_select "$tin"
		json_get_vars threshold_rate sent_bytes sent_packets backlog_bytes target_us peak_delay_us avg_delay_us base_delay_us drops ecn_mark ack_drops sparse_flows bulk_flows unresponsive_flows

		eval osp="\$osp${ifr}t${i}"
		if  [ "$osp" ] && [ "$osp" -eq "$sent_packets" ] ; then
			peak_delay_us=0; avg_delay_us=0; base_delay_us=0
			sparse_flows=0; bulk_flows=0; unresponsive_flows=0
		else
			eval "osp${ifr}t${i}=$sent_packets"
		fi

		tn=${tns:$((i<<1)):2}

		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_bytes-$tn\" interval=$INTERVAL N:$sent_bytes"
		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_thres-$tn\" interval=$INTERVAL N:$threshold_rate"
		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_drops-$tn\" interval=$INTERVAL N:$drops:$ecn_mark:$ack_drops"
		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_backlog-$tn\" interval=$INTERVAL N:$backlog_bytes"
		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_flows-$tn\" interval=$INTERVAL N:$sparse_flows:$bulk_flows:$unresponsive_flows"
		echo "PUTVAL \"$HOSTNAME/sqmcake-$ifc/qdisct_latencyus-$tn\" interval=$INTERVAL N:$target_us:$peak_delay_us:$avg_delay_us:$base_delay_us"

		json_select ..
		i=$((i+1))
	done
	json_select ..
}

handle_mq() {
	ifc="$1"

	# Overall
	json_get_vars bytes drops backlog

	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_bytes\" interval=$INTERVAL N:$bytes"
	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_drops\" interval=$INTERVAL N:$drops"
	echo "PUTVAL \"$HOSTNAME/sqm-$ifc/qdisc_backlog\" interval=$INTERVAL N:$backlog"
}

process_qdisc() {
	local ifc jsn

	ifc="$1"
	jsn=$(tc -s -j qdisc show dev "$ifc") || return

	# strip leading & trailing []
	jsn="${jsn#[}" ; jsn="${jsn%]}"

	json_load "${jsn}"
	json_get_var qdisc kind

	case "$qdisc" in
		cake) handle_cake "$ifc"
		;;
		mq) handle_mq "$ifc"
		;;

		*) echo "Unknown qdisc type '$qdisc' on interface '$ifc'" 1>&2
		;;
	esac
	json_cleanup
}

while true ; do
	for ifc in "$@" ; do
		process_qdisc "$ifc"
	done
	sleep "${INTERVAL%%.*}"
done
