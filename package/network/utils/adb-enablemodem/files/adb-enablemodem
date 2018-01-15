#!/bin/sh /etc/rc.common

START=99

adb_exec() {
	adb -s "$serial" shell "( $1 ) >/dev/null 2>&1"'; printf "\nEXIT_CODE: %i\n" $?' | head -c 64 | grep -qx 'EXIT_CODE: 0\r\?'
}

enablemodem_do() {
	logger -t adb-enablemodem 'INFO: waiting for device'
	adb wait-for-device
	serial="$(adb get-serialno)"

	vendor_id="$(adb -s "$serial" shell 'uci get product.usb.vid' | head -c 16 | tr -d '\r\n')"
	product_id="$(adb -s "$serial" shell 'uci get product.usb.pid' | head -c 16 | tr -d '\r\n')"

	case "$vendor_id:$product_id" in
	"0x2357:0x000D") # TP-LINK LTE MODULE
		case "$1" in
		start)
			if adb_exec '
				chmod +x /WEBSERVER/www/cgi-bin/*
				fds="$(ls /proc/$$/fd | grep -v "^[012]$")"
				for fd in $fds; do
					eval "exec $fd>&-"
				done
				start-stop-daemon -x httpd -S -- -h /WEBSERVER/www/
			'; then
				logger -t adb-enablemodem 'INFO: httpd on modem started'
			else
				logger -t adb-enablemodem 'ERROR: failed to start httpd on modem'
			fi
			option_newid='/sys/bus/usb-serial/drivers/option1/new_id'
			if [ -e "$option_newid" ]; then
				printf '%s %s' "$vendor_id" "$product_id" > "$option_newid"
			fi
			;;
		stop)
			if adb_exec 'start-stop-daemon -x httpd -K'; then
				logger -t adb-enablemodem 'INFO: httpd on modem stopped'
			else
				logger -t adb-enablemodem 'ERROR: failed to stop httpd on modem'
			fi
			;;
		esac
		;;
	*)
		logger -t adb-enablemodem "ERROR: unknown device $vendor_id:$product_id"
		;;
	esac
}

start() {
	( enablemodem_do start ) &
}

stop() {
	( enablemodem_do stop ) &
}

restart() {
	( enablemodem_do stop; enablemodem_do start ) &
}

