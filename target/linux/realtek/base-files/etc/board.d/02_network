
. /lib/functions.sh
. /lib/functions/uci-defaults.sh
. /lib/functions/system.sh

_filter_port_list_ordered() {
	local ports="$1"
	local excluded="$2"
	local sort_opts="$3"
	echo $ports $excluded | xargs -n1 basename | sort -V $sort_opts | uniq -u | xargs
}

filter_port_list() {
	_filter_port_list_ordered "$1" "$2"
}

filter_port_list_reverse() {
	_filter_port_list_ordered "$1" "$2" "-r"
}

realtek_setup_interfaces()
{
	local board="$1"
	local lan_list="$2"

	ucidef_set_bridge_device switch
	ucidef_set_interface_lan "$lan_list"
}

realtek_setup_macs_lan()
{
	local lan_list="$1"
	local lan_mac_start="$2"
	local lan_mac_end="$3"

	for lan in $lan_list; do
		ucidef_set_network_device_mac $lan $lan_mac_start
		[ -z "$lan_mac_end" ] || [ "$lan_mac_start" == "$lan_mac_end" ] && lan_mac_start=$(macaddr_setbit_la $lan_mac_start)
		lan_mac_start=$(macaddr_add $lan_mac_start 1)
	done
}

realtek_setup_macs()
{
	local board="$1"
	local lan_list="$2"

	local eth0_mac=""
	local lan_mac=""
	local lan_mac_start=""
	local lan_mac_end=""
	local label_mac=""

	case $board in
	datto,l8|\
	edgecore,ecs2100-52t|\
	edgecore,ecs4100-12ph|\
	linksys,lgs310c|\
	linksys,lgs328c|\
	linksys,lgs328mpc-v2|\
	linksys,lgs352c|\
	netgear,gs108t-v3|\
	netgear,gs110tpp-v1|\
	netgear,gs110tup-v1|\
	netgear,gs308t-v1|\
	netgear,gs310tp-v1|\
	tplink,sg2008p-v1|\
	tplink,sg2008p-v3|\
	tplink,sg2210p-v3|\
	tplink,sg2452p-v4|\
	tplink,t1600g-28ts-v3|\
	xikestor,sks8300-8t|\
	xikestor,sks8300-12e2t2x|\
	xikestor,sks8300-12x-v1|\
	xikestor,sks8310-8x|\
	zyxel,xgs1210-12-a1|\
	zyxel,xgs1210-12-b1)
		lan_mac=$(get_mac_label)
		lan_mac_start=$lan_mac
		;;
	d-link,dgs-1250-28x|\
	hasivo,s1100wp-8gt-se|\
	hasivo,s600wp-5gt-2sx-se|\
	hpe,1920-8g|\
	hpe,1920-8g-poe-65w|\
	hpe,1920-8g-poe-180w|\
	hpe,1920-16g|\
	hpe,1920-24g|\
	hpe,1920-24g-poe-180w|\
	hpe,1920-24g-poe-370w|\
	hpe,1920-48g|\
	hpe,1920-48g-poe|\
	plasmacloud,esx28|\
	plasmacloud,mcx3|\
	plasmacloud,psx8|\
	plasmacloud,psx10|\
	plasmacloud,psx28|\
	sirivision,sr-st3408f|\
	ubnt,usw-pro-xg-8-poe|\
	zyxel,gs1920-24hp-v2)
		lan_mac="$(get_mac_label)"
		;;
	hasivo,f1100w-4sx-4xgt|\
	hasivo,f1100w-4sx-4xgt-512mb|\
	hasivo,f1100wp-4sx-4xgt|\
	hasivo,f1100wp-4sx-4xgt-512mb|\
	hasivo,f5800w-12s-plus|\
	hasivo,s1300wp-8xgt-4s-plus|\
	hasivo,s600wp-5gt-2s-plus|\
	mokerlink,10gt080m|\
	tplink,tl-st1008f-v2|\
	zyxel,xgs1010-12-a1|\
	zyxel,xgs1010-12-b1)
		lan_mac=$(mtd_get_mac_ascii u-boot-env ethaddr)
		mac_prefix=$(echo "$lan_mac" | cut -d: -f1-5)
		[ -z "$lan_mac" ] || [ "$mac_prefix" = "00:e0:4c:00:00" ] && lan_mac=$(macaddr_random)
		lan_mac_start=$lan_mac
		eth0_mac=$lan_mac
		;;
	xikestor,sks8300-8x)
		lan_mac=$(mtd_get_mac_binary board-info 0x1f1)
		lan_mac_start=$lan_mac
		eth0_mac=$lan_mac
		;;
	allnet,all-sg8208m|\
	apresia,aplgs120gtss|\
	d-link,dgs-1210-10mp-f|\
	d-link,dgs-1210-10p|\
	d-link,dgs-1210-16|\
	d-link,dgs-1210-20|\
	d-link,dgs-1210-26|\
	d-link,dgs-1210-28|\
	d-link,dgs-1210-28mp-f|\
	d-link,dgs-1210-28p-f|\
	d-link,dgs-1210-52|\
	engenius,ews2910p-v1|\
	engenius,ews2910p-v3|\
	hasivo,s1100w-8xgt-se|\
	hasivo,s1100wp-8xgt-se|\
	inaba,aml2-17gp|\
	iodata,bsh-g24mb|\
	netgear,gs750e|\
	panasonic,m16eg-pn28160k|\
	panasonic,m24eg-pn28240k|\
	panasonic,m48eg-pn28480k|\
	panasonic,m8eg-pn28080k|\
	vimin,vm-s100-0800ms|\
	zyxel,gs1900-10hp-a1|\
	zyxel,gs1900-10hp-b1|\
	zyxel,gs1900-16-a1|\
	zyxel,gs1900-24-a1|\
	zyxel,gs1900-24-b1|\
	zyxel,gs1900-24e-a1|\
	zyxel,gs1900-24e-b1|\
	zyxel,gs1900-24ep-a1|\
	zyxel,gs1900-24hp-a1|\
	zyxel,gs1900-24hp-b1|\
	zyxel,gs1900-48-a1|\
	zyxel,gs1900-48hp-a1|\
	zyxel,gs1900-8-a1|\
	zyxel,gs1900-8-b1|\
	zyxel,gs1900-8hp-a1|\
	zyxel,gs1900-8hp-b1|\
	zyxel,gs1900-8hp-b2|\
	zyxel,gs1920-24hp-v1|\
	zyxel,xgs1250-12-a1|\
	zyxel,xgs1250-12-b1)
		lan_mac=$(mtd_get_mac_ascii u-boot-env2 mac_start)
		lan_mac_end=$(mtd_get_mac_ascii u-boot-env2 mac_end)
		label_mac=$(mtd_get_mac_ascii u-boot-env ethaddr)
		[ -z "$lan_mac" ] && lan_mac=$label_mac
		lan_mac_start=$lan_mac
		eth0_mac=$lan_mac
		;;
	esac

	[ -n "$eth0_mac" ] && ucidef_set_network_device_mac eth0 $eth0_mac

	[ -n "$lan_mac" ] && ucidef_set_interface_macaddr "lan" $lan_mac
	[ -n "$lan_mac" ] && ucidef_set_bridge_mac "$lan_mac"

	[ -n "$lan_mac_start" ] && realtek_setup_macs_lan "$lan_list" "$lan_mac_start" "$lan_mac_end"

	[ -n "$label_mac" ] && ucidef_set_label_macaddr $label_mac
}

realtek_setup_poe()
{
	local board="$1"
	local lan_list="$2"

	case $board in
	d-link,dgs-1210-10mp-f)
		ucidef_set_poe 130 "$(filter_port_list "$lan_list" "lan9 lan10")"
		;;
	d-link,dgs-1210-10p)
		ucidef_set_poe 65 "$(filter_port_list "$lan_list" "lan9 lan10")"
		;;
	d-link,dgs-1210-28mp-f)
		ucidef_set_poe 370 "lan8 lan7 lan6 lan5 lan4 lan3 lan2 lan1 lan16 lan15 lan14 lan13 lan12 lan11 lan10 lan9 lan24 lan23
				lan22 lan21 lan20 lan19 lan18 lan17"
		;;
	d-link,dgs-1210-28p-f)
		ucidef_set_poe 193 "lan8 lan7 lan6 lan5 lan4 lan3 lan2 lan1 lan16 lan15 lan14 lan13 lan12 lan11 lan10 lan9 lan24 lan23
				lan22 lan21 lan20 lan19 lan18 lan17"
		;;
	edgecore,ecs4100-12ph|\
	netgear,gs110tpp-v1)
		ucidef_set_poe 130 "$(filter_port_list "$lan_list" "lan9 lan10")"
		;;
	datto,l8)
		ucidef_set_poe 55 "lan5 lan6 lan7 lan8 lan4 lan3 lan2 lan1"
		;;
	hpe,1920-8g-poe-65w)
		ucidef_set_poe 65 "$(filter_port_list_reverse "$lan_list" "lan9 lan10")"
		;;
	hpe,1920-8g-poe-180w)
		ucidef_set_poe 180 "$(filter_port_list_reverse "$lan_list" "lan9 lan10")"
		;;
	hpe,1920-24g-poe-180w)
		ucidef_set_poe 180 "$(filter_port_list_reverse "$lan_list" "lan25 lan26 lan27 lan28")"
		;;
	hpe,1920-24g-poe-370w)
		ucidef_set_poe 370 "$(filter_port_list_reverse "$lan_list" "lan25 lan26 lan27 lan28")"
		;;
	hpe,1920-48g-poe)
		ucidef_set_poe 370 "lan8 lan7 lan6 lan5 lan4 lan3 lan2 lan1 lan16 lan15 lan14 lan13 lan12 lan11 lan10 lan9 lan24 lan23
				lan22 lan21 lan20 lan19 lan18 lan17 lan32 lan31 lan30 lan29 lan28 lan27 lan26 lan25 lan40 lan39 lan38 lan37
				lan36 lan35 lan34 lan33 lan48 lan47 lan46 lan45 lan44 lan43 lan42 lan41"
		;;
	netgear,gs110tup-v1)
		ucidef_set_poe 240 "$(filter_port_list "$lan_list" "lan9 lan10")"
		;;
	netgear,gs310tp-v1)
		ucidef_set_poe 55 "$(filter_port_list "$lan_list" "lan9 lan10")"
		;;
	zyxel,gs1900-24ep-a1)
		ucidef_set_poe 130 "lan1 lan2 lan3 lan4 lan5 lan6 lan7 lan8 lan9 lan10 lan11 lan12"
		;;
	zyxel,gs1900-24hp-a1|\
	zyxel,gs1900-24hp-b1)
		ucidef_set_poe 170 "$(filter_port_list "$lan_list" "lan25 lan26")"
		;;
	zyxel,gs1900-48hp-a1)
		ucidef_set_poe 170 "lan1 lan2 lan3 lan4 lan5 lan6 lan7 lan8 \
			lan9 lan10 lan11 lan12 lan13 lan14 lan15 lan16 \
			lan17 lan18 lan19 lan20 lan21 lan22 lan23 lan24"
		;;
	esac
}

realtek_lan_list()
{
	local labels

	labels="$(find /sys/firmware/devicetree/base -path '*/ethernet-ports/port@*/label' 2>/dev/null | \
		xargs cat 2>/dev/null | tr '\0' '\n' | grep '^lan' | sort -V | uniq | xargs)"

	[ -n "$labels" ] || labels="$(ls -1 -v -d /sys/class/net/lan* 2>/dev/null | xargs -n1 basename | xargs)"

	echo "$labels"
}

board=$(board_name)
board_config_update
lan_list=$(realtek_lan_list)
realtek_setup_interfaces "$board" "$lan_list"
realtek_setup_macs "$board" "$lan_list"
realtek_setup_poe "$board" "$lan_list"
board_config_flush

exit 0
