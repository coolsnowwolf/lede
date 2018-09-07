#
# Copyright (C) 2010 OpenWrt.org
#

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	3g150b|\
	3g300m|\
	a5-v11|\
	ai-br100|\
	air3gii|\
	alfa-network,ac1200rm|\
	alfa-network,awusfree1|\
	all0239-3g|\
	all0256n-4M|\
	all0256n-8M|\
	all5002|\
	all5003|\
	mediatek,ap-mt7621a-v60|\
	ar725w|\
	asl26555-8M|\
	asl26555-16M|\
	awapn2403|\
	awm002-evb-4M|\
	awm002-evb-8M|\
	bc2|\
	bocco|\
	broadway|\
	c108|\
	carambola|\
	cf-wr800n|\
	cs-qr10|\
	d105|\
	d240|\
	dap-1350|\
	db-wrt01|\
	dcs-930|\
	dcs-930l-b1|\
	dir-300-b1|\
	dir-300-b7|\
	dir-320-b1|\
	dir-600-b1|\
	dir-615-d|\
	dir-615-h1|\
	dir-620-a1|\
	dir-620-d1|\
	dir-810l|\
	duzun-dm06|\
	e1700|\
	elecom,wrc-1167ghbk2-s|\
	elecom,wrc-2533gst|\
	elecom,wrc-1900gst|\
	esr-9753|\
	ew1200|\
	ex2700|\
	ex3700|\
	f7c027|\
	firewrt|\
	fonera20n|\
	freestation5|\
	gnubee,gb-pc1|\
	gnubee,gb-pc2|\
	gl-mt300a|\
	gl-mt300n|\
	gl-mt750|\
	gl-mt300n-v2|\
	hc5*61|\
	hc5661a|\
	hg255d|\
	hiwifi,hc5861b|\
	hlk-rm04|\
	hpm|\
	ht-tm02|\
	hw550-3g|\
	iodata,wn-ax1167gr|\
	iodata,wn-gx300gr|\
	ip2202|\
	jhr-n805r|\
	jhr-n825r|\
	jhr-n926r|\
	k2p|\
	kimax,u35wf|\
	kn|\
	kn_rc|\
	kn_rf|\
	kng_rc|\
	linkits7688|\
	m2m|\
	m3|\
	m4-4M|\
	m4-8M|\
	mac1200rv2|\
	microwrt|\
	miniembplug|\
	miniembwifi|\
	miwifi-mini|\
	miwifi-nano|\
	mlw221|\
	mlwg2|\
	mofi3500-3gn|\
	mpr-a1|\
	mpr-a2|\
	mr-102n|\
	mt7628|\
	mzk-750dhp|\
	mzk-dp150n|\
	mzk-ex300np|\
	mzk-ex750np|\
	mzk-w300nh2|\
	mzk-wdpr|\
	nbg-419n|\
	nbg-419n2|\
	newifi-d1|\
	d-team,newifi-d2|\
	nixcore-x1-8M|\
	nixcore-x1-16M|\
	nw718|\
	omega2|\
	omega2p|\
	oy-0001|\
	pbr-d1|\
	pbr-m1|\
	phicomm,k2g|\
	psg1208|\
	psg1218a|\
	psg1218b|\
	psr-680w|\
	px-4885-4M|\
	px-4885-8M|\
	netgear,r6120|\
	rb750gr3|\
	re6500|\
	rp-n53|\
	rt5350f-olinuxino|\
	rt5350f-olinuxino-evb|\
	rt-ac51u|\
	rt-g32-b1|\
	rt-n10-plus|\
	rt-n12p|\
	rt-n13u|\
	rt-n14u|\
	rt-n15|\
	rt-n56u|\
	rut5xx|\
	sap-g3200u3|\
	sk-wb8|\
	sl-r7205|\
	tama,w06|\
	tew-638apb-v2|\
	tew-691gr|\
	tew-692gr|\
	tew-714tru|\
	timecloud|\
	tiny-ac|\
	u25awf-h1|\
	u7621-06-256M-16M|\
	u7628-01-128M-16M|\
	ur-326n4g|\
	ur-336un|\
	v22rw-2x2|\
	vonets,var11n-300|\
	vocore-8M|\
	vocore-16M|\
	vocore2|\
	vocore2lite|\
	vr500|\
	w150m|\
	w2914nsv2|\
	w306r-v20|\
	w502u|\
	ravpower,wd03|\
	wf-2881|\
	whr-1166d|\
	whr-300hp2|\
	whr-600d|\
	whr-g300n|\
	widora,neo-16m|\
	widora,neo-32m|\
	mqmaker,witi-256m|\
	mqmaker,witi-512m|\
	wizfi630a|\
	wl-330n|\
	wl-330n3g|\
	wl-341v3|\
	wl-351|\
	wl-wn575a3|\
	wli-tx4-ag300n|\
	wlr-6000|\
	wmdr-143n|\
	wmr-300|\
	wn3000rpv3|\
	wnce2001|\
	wndr3700v5|\
	wr512-3gn-4M|\
	wr512-3gn-8M|\
	wr6202|\
	wrh-300cr|\
	wrtnode|\
	wrtnode2r |\
	wrtnode2p |\
	wsr-600|\
	wt1520-4M|\
	wt1520-8M|\
	wt3020-4M|\
	wt3020-8M|\
	wzr-agl300nh|\
	x5|\
	x8|\
	y1|\
	y1s|\
	youhua,wr1200js|\
	we1026-5g-16m|\
	zbt-ape522ii|\
	zbt-cpe102|\
	zbt-wa05|\
	zbtlink,zbt-we1226|\
	zbt-we1326|\
	zbt-we2026|\
	zbtlink,zbt-we3526|\
	zbt-we826-16M|\
	zbt-we826-32M|\
	zbt-wg2626|\
	zbt-wg3526-16M|\
	zbt-wg3526-32M|\
	zbt-wr8305rt|\
	zorlik,zl5900v2|\
	zte-q7|\
	zyxel,keenetic-extra-ii|\
	youku-yk1)
		[ "$magic" != "27051956" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	3g-6200n|\
	3g-6200nl|\
	br-6475nd|\
	edimax,br-6478ac-v2)
		[ "$magic" != "43535953" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;

	ar670w)
		[ "$magic" != "6d000080" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	c20i|\
	c50|\
	mr200|\
	tplink,c2-v1|\
	tplink,c20-v1|\
	tplink,c20-v4|\
	tplink,c50-v3|\
	tplink,tl-mr3020-v3|\
	tplink,tl-mr3420-v5|\
	tplink,tl-wa801nd-v5|\
	tplink,tl-wr842n-v5|\
	tplink,tl-wr902ac-v3|\
	tl-wr840n-v4|\
	tl-wr840n-v5|\
	tl-wr841n-v13)
		[ "$magic" != "03000000" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	cy-swr1100|\
	dch-m225|\
	dir-610-a1|\
	dir-645|\
	dir-860l-b1)
		[ "$magic" != "5ea3a417" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	dlink,dwr-116-a1|\
	dlink,dwr-118-a2|\
	dlink,dwr-921-c1|\
	dwr-512-b)
		[ "$magic" != "0404242b" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	hc5962|\
	mir3g|\
	r6220|\
	ubnt-erx|\
	ubnt-erx-sfp)
		nand_do_platform_check "$board" "$1"
		return $?;
		;;
	mikrotik,rbm11g|\
	mikrotik,rbm33g|\
	re350-v1)
		[ "$magic" != "01000000" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	wcr-1166ds|\
	wsr-1166)
		[ "$magic" != "48445230" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	mikrotik,rbm11g|\
	mikrotik,rbm33g)
		[ -z "$(rootfs_type)" ] && mtd erase firmware
		;;
	esac
}

platform_nand_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	ubnt-erx|\
	ubnt-erx-sfp)
		platform_upgrade_ubnt_erx "$ARGV"
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	hc5962|\
	mir3g|\
	r6220|\
	ubnt-erx|\
	ubnt-erx-sfp)
		nand_do_upgrade "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
