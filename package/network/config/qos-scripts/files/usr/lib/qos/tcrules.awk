BEGIN {
	dmax=100
	if (!(linespeed > 0)) linespeed = 128
	FS=":"
	n = 0
}

($1 != "") {
	n++
	class[n] = $1
	prio[n] = $2
	avgrate[n] = ($3 * linespeed / 100)
	pktsize[n] = $4
	delay[n] = $5
	maxrate[n] = ($6 * linespeed / 100)
	qdisc[n] = $7
	filter[n] = $8
}

END {
	allocated = 0
	maxdelay = 0

	for (i = 1; i <= n; i++) {
		# set defaults
		if (!(pktsize[i] > 0)) pktsize[i] = 1500
		if (!(prio[i] > 0)) prio[i] = 1

		allocated += avgrate[i]
		sum_prio += prio[i]
		if ((avgrate[i] > 0) && !(delay[i] > 0)) {
			sum_rtprio += prio[i]
		}
	}

	# allocation of m1 in rt classes:
	# sum(d * m1) must not exceed dmax * (linespeed - allocated)
	dmax = 0
	for (i = 1; i <= n; i++) {
		if (avgrate[i] > 0) {
			rtm2[i] = avgrate[i]
			if (delay[i] > 0) {
				d[i] = delay[i]
			} else {
				d[i] = 2 * pktsize[i] * 1000 / (linespeed * 1024)
				if (d[i] > dmax) dmax = d[i]
			}
		}
	}

	ds_avail = dmax * (linespeed - allocated)
	for (i = 1; i <= n; i++) {
		lsm1[i] = 0
		rtm1[i] = 0
		lsm2[i] = linespeed * prio[i] / sum_prio
		if ((avgrate[i] > 0) && (d[i] > 0)) {
			if (!(delay[i] > 0)) {
				ds = ds_avail * prio[i] / sum_rtprio
				ds_avail -= ds
				rtm1[i] = rtm2[i] + ds/d[i]
			}
			lsm1[i] = rtm1[i]
		}
		else {
			d[i] = 0
		}
	}

	# main qdisc
	for (i = 1; i <= n; i++) {
		printf "tc class add dev "device" parent 1:1 classid 1:"class[i]"0 hfsc"
		if (rtm1[i] > 0) {
			printf " rt m1 " int(rtm1[i]) "kbit d " int(d[i] * 1000) "us m2 " int(rtm2[i])"kbit"
		}
		printf " ls m1 " int(lsm1[i]) "kbit d " int(d[i] * 1000) "us m2 " int(lsm2[i]) "kbit"
		print " ul rate " int(maxrate[i]) "kbit"
	}

	# leaf qdisc
	avpkt = 1200
	for (i = 1; i <= n; i++) {
		print "tc qdisc add dev "device" parent 1:"class[i]"0 handle "class[i]"00: fq_codel limit 800 quantum 300 noecn"
	}

	# filter rule
	for (i = 1; i <= n; i++) {
		filter_cmd = "tc filter add dev "device" parent 1: prio %d handle %s fw flowid 1:%d0\n";
		if (direction == "up") {
			filter_1 = sprintf("0x%x0/0xf0", class[i])
			filter_2 = sprintf("0x0%x/0x0f", class[i])
		} else {
			filter_1 = sprintf("0x0%x/0x0f", class[i])
			filter_2 = sprintf("0x%x0/0xf0", class[i])
		}

		printf filter_cmd, class[i] * 2, filter_1, class[i]
		printf filter_cmd, class[i] * 2 + 1, filter_2, class[i]

		filterc=1
		if (filter[i] != "") {
			print " tc filter add dev "device" parent "class[i]"00: handle "filterc"0 "filter[i]
			filterc=filterc+1
		}
	}
}
