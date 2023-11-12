#!/bin/sh

listen_address=$1
listen_port=$2
server_address=$3
server_port=$4
status=$(/usr/bin/curl -I -o /dev/null -skL -x socks5h://${server_address}:${server_port} --connect-timeout 3 --retry 3 -w %{http_code} "https://www.google.com/generate_204")
case "$status" in
	204|\
	200)
		status=200
	;;
esac
return_code=1
if [ "$status" = "200" ]; then
	return_code=0
fi
exit ${return_code}
