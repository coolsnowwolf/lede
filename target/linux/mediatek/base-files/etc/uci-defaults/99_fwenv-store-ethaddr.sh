[ ! -e /etc/fw_env.config ] && exit 0

. /lib/functions/system.sh

case "$(board_name)" in
bananapi,bpi-r2|\
bananapi,bpi-r64|\
unielec,u7623-02)
	[ -z "$(fw_printenv -n ethaddr 2>/dev/null)" ] &&
		fw_setenv ethaddr "$(cat /sys/class/net/eth0/address)"
	;;
bananapi,bpi-r3)
	[ -z "$(fw_printenv -n ethaddr 2>/dev/null)" ] &&
		fw_setenv ethaddr "$(cat /sys/class/net/eth0/address)"
	[ -z "$(fw_printenv -n eth1addr 2>/dev/null)" ] &&
		fw_setenv eth1addr "$(macaddr_add $(cat /sys/class/net/eth0/address) 1)"
	;;
esac

exit 0
