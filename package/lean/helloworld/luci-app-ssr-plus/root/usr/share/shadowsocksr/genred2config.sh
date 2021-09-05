#!/bin/sh
argv1=$1
argv2=$2
argv3=$3
argv4=$4
argv5=$5
argv6=$6
argv7=$7
argv8=$8
argv9=$9
cat <<-EOF >$argv1
	base {
	log_debug = off;
	log_info = off;
	log = stderr;
	daemon = on;
	redirector = iptables;
	reuseport = on;
	}
EOF
tcp() {
	if [ "$argv7" == "0" ]; then
		cat <<-EOF >>$argv1
			redsocks {
			bind = "0.0.0.0:$argv4";
			relay = "$argv5:$argv6";
			type = socks5;
			autoproxy = 0;
			timeout = 10;
			}
		EOF
	else
		cat <<-EOF >>$argv1
			redsocks {
			bind = "0.0.0.0:$argv4";
			relay = "$argv5:$argv6";
			type = socks5;
			autoproxy = 0;
			timeout = 10;
			login = "$argv8";
			password = "$argv9";
			}
		EOF
	fi
}
udp() {
	if [ "$argv7" == "0" ]; then
		cat <<-EOF >>$argv1
			redudp {
			bind = "0.0.0.0:$argv4";
			relay = "$argv5:$argv6";
			type = socks5;
			udp_timeout = 10;
			}
		EOF
	else
		cat <<-EOF >>$argv1
			redudp {
			bind = "0.0.0.0:$argv4";
			relay = "$argv5:$argv6";
			type = socks5;
			udp_timeout = 10;
			login = "$argv8";
			password = "$argv9";
			}
		EOF
	fi
}
case "$argv2" in
socks5)
	case "$argv3" in
	tcp)
		tcp
		;;
	udp)
		udp
		;;
	*)
		tcp
		udp
		;;
	esac
	;;
*)
	cat <<-EOF >>$argv1
		redsocks {
		bind = "0.0.0.0:$argv4";
		type = direct;
		interface = $argv3;
		autoproxy = 0;
		timeout = 10;
		}
	EOF
	;;
esac
