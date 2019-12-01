#!/bin/bash -e

#
# Script for generating China IPv4 route table by merging APNIC.net data and IPIP.net data
#

china_routes_ipip()
{
	[ -f ipip.txt ] || wget -4 https://cdn.jsdelivr.net/gh/17mon/china_ip_list/china_ip_list.txt -O ipip.txt >&2 || exit 1
	cat ipip.txt | xargs netmask | awk '{print $1}'
}

china_routes_apnic()
{
	[ -f apnic.txt ] || wget -4 http://ftp.apnic.net/stats/apnic/delegated-apnic-latest -O apnic.txt >&2 || exit 1

	cat apnic.txt | awk -F'|' -vc=CN '
function tobits(c) { for(n=0; c>=2; c/=2) n++; return 32-n; }
$2==c&&$3=="ipv4" { printf("%s/%d\n", $4, tobits($5)) }' |
		xargs netmask | awk '{print $1}'
}

china_routes_merged()
{
	[ -x ./ipv4-merger ] || gcc ipv4_merger.c -o ipv4-merger >&2

	china_routes_apnic > china.apnic
	china_routes_ipip > china.ipip

	# Merge them together
	cat china.apnic china.ipip | ./ipv4-merger | sed 's/\-/:/g' |
		xargs netmask | awk '{print $1}' | awk -F/ '$2<=24' > china.merged

	cat china.merged
}

# $1: ipset name
convert_routes_to_ipset()
{
	local ipset_name="$1"
	echo "create $ipset_name hash:net family inet hashsize 1024 maxelem 65536"
	awk -vt="$ipset_name" '{ printf("add %s %s\n", t, $0) }'
}


generate_china_ipset()
{
	china_routes_merged | convert_routes_to_ipset china
}

generate_inverted_china_routes()
{
	(
		china_routes_merged
		echo 0.0.0.0/8 10.0.0.0/8 100.64.0.0/10 127.0.0.0/8 172.16.0.0/12 192.168.0.0/16 224.0.0.0/3
	) |
	xargs netmask -r | awk '{print $1}' |
	awk -F- '
function iptoint(ip) { split(ip,arr,"."); n=0; for(i=1;i<=4;i++) n=n*256+arr[i]; return n; }
function inttoip(n) { a=int(n/16777216); b=int(n%16777216/65536); c=int(n%65536/256); d=n%256; return a "." b "." c "." d; }
BEGIN { st=0 }
{ x=st; y=iptoint($1); st=iptoint($2)+1; if(y>x) { print inttoip(x) ":" inttoip(y-1); } }' |
	xargs netmask | awk '{print $1}'
}


##
case "$1" in
	"")
		generate_china_ipset
		;;
	-c)
		china_routes_merged
		;;
	-r)
		generate_inverted_china_routes
		;;
	*)
		echo "Usage:"
		echo " $0                   generate China routes in ipset format"
		echo " $0 -c                generate China routes in IP/prefix format"
		echo " $0 -r                generate invert China routes"
		;;
	*)
esac
