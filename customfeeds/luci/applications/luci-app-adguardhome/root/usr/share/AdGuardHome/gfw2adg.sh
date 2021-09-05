#!/bin/sh

PATH="/usr/sbin:/usr/bin:/sbin:/bin"

checkmd5(){
local nowmd5=$(md5sum /tmp/adguard.list 2>/dev/null)
nowmd5=${nowmd5%% *}
local lastmd5=$(uci get AdGuardHome.AdGuardHome.gfwlistmd5 2>/dev/null)
if [ "$nowmd5" != "$lastmd5" ]; then
	uci set AdGuardHome.AdGuardHome.gfwlistmd5="$nowmd5"
	uci commit AdGuardHome
	[ "$1" == "noreload" ] || /etc/init.d/AdGuardHome reload
fi
}

configpath=$(uci get AdGuardHome.AdGuardHome.configpath 2>/dev/null)
[ "$1" == "del" ] && sed -i '/programaddstart/,/programaddend/d' $configpath && checkmd5 "$2" && exit 0
gfwupstream=$(uci get AdGuardHome.AdGuardHome.gfwupstream 2>/dev/null)
if [ -z $gfwupstream ]; then
gfwupstream="tcp://208.67.220.220:5353"
fi
if [ ! -f "$configpath" ]; then
	echo "please make a config first"
	exit 1
fi
wget-ssl --no-check-certificate https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt -O- | base64 -d > /tmp/gfwlist.txt
cat /tmp/gfwlist.txt | awk -v upst="$gfwupstream" 'BEGIN{getline;}{
s1=substr($0,1,1);
if (s1=="!")
{next;}
if (s1=="@"){
    $0=substr($0,3);
    s1=substr($0,1,1);
    white=1;}
else{
    white=0;
}

if (s1=="|")
    {s2=substr($0,2,1);
    if (s2=="|")
    {
        $0=substr($0,3);
        split($0,d,"/");
        $0=d[1];
    }else{
        split($0,d,"/");
        $0=d[3];
    }}
else{
    split($0,d,"/");
    $0=d[1];
}
star=index($0,"*");
if (star!=0)
{
    $0=substr($0,star+1);
    dot=index($0,".");
    if (dot!=0)
        $0=substr($0,dot+1);
    else
        next;
    s1=substr($0,1,1);
}
if (s1==".")
{fin=substr($0,2);}
else{fin=$0;}
if (index(fin,".")==0) next;
if (index(fin,"%")!=0) next;
if (index(fin,":")!=0) next;
match(fin,"^[0-9\.]+")
if (RSTART==1 && RLENGTH==length(fin)) {print "ipset add gfwlist "fin>"/tmp/doipset.sh";next;}
if (fin=="" || finl==fin) next;
finl=fin;
if (white==0)
    {print("  - '\''[/"fin"/]"upst"'\''");}
else{
    print("  - '\''[/"fin"/]#'\''");}
}END{print("  - '\''[/programaddend/]#'\''")}' > /tmp/adguard.list
grep programaddstart $configpath
if [ "$?" == "0" ]; then
	sed -i '/programaddstart/,/programaddend/c\  - '\''\[\/programaddstart\/\]#'\''' $configpath
	sed -i '/programaddstart/'r/tmp/adguard.list $configpath
else
	sed -i '1i\  - '\''[/programaddstart/]#'\''' /tmp/adguard.list
	sed -i '/upstream_dns:/'r/tmp/adguard.list $configpath
fi
checkmd5 "$2"
rm -f /tmp/gfwlist.txt /tmp/adguard.list
