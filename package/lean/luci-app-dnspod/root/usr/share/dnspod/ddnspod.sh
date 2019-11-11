#!/bin/bash

#################################################
# AnripDdns v5.08
# Dynamic DNS using DNSPod API
# Original by anrip<mail@anrip.com>, http://www.anrip.com/ddnspod
# Edited by ProfFan
# Edited by dz
#################################################
enable=1
sleeptime=60
ip_mode=2
WanIp=""
LastIpFile=/usr/share/dnspod/last.ip
logfile="/var/log/ddnspod.log"
sleeptime=$(uci get dnspod.base_arg.time 2>/dev/null)
enable=$(uci get dnspod.base_arg.enabled 2>/dev/null)
IPtype=$(uci get dnspod.@ip_last[0].IPtype 2>/dev/null)
iface=$(uci get dnspod.@ip_last[0].iface 2>/dev/null)
ipv6iface=$(uci get dnspod.@ip_last[0].ipv6iface 2>/dev/null)
linenumber=$(uci get dnspod.@ip_last[0].linenumber 2>/dev/null)

clean_log(){
logrow=$(grep -c "" ${logfile})
if [ $logrow -ge 100 ];then
    cat /dev/null > ${logfile}
    echo "$(date "+%Y-%m-%d %H:%M:%S") Log条数超限，清空处理！" >> ${logfile}
fi

logrow1=$(grep -c "" ${logfile1})
if [ $logrow1 -ge 300 ];then
    cat /dev/null > ${logfile1}
    echo "Log条数超限，清空处理！" >> ${logfile1}
fi
}

while [ "$enable" = "1" ];
do

clean_log

function getPublicIp() {
	record_type=$record_type
	if [ $record_type = 'AAAA' ]; then
    case $IPtype in
        '1')
            WanIp=`curl -6 -s 'https://api.myip.com' | grep ip |sed 's/.*ip":"\([^"]*\)".*/\1/g'`
            echo $WanIp
            ;;
        '2')
            WanIp=`curl -6 -s 'http://ip-api.com/json/?fields=query' | grep query |sed 's/.*query":"\([^"]*\)".*/\1/g'`
            echo $WanIp
            ;;
        '3')
			WanIp=`curl -6 -s 'https://ipapi.co/ip/'`
            echo $WanIp
            ;;
        '4')
            WanIp=`curl -6 -s 'https://ifconfig.co'`
            echo $WanIp
            ;;
        '5')
            WanIp=`ubus call network.interface.$ipv6iface status | grep '"address"' | grep -oE '([a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7})' |sed -n ''$linenumber''`
            echo $WanIp
            ;;
        '6')
            WanIp=`curl -6 -s 'https://ip6.seeip.org'`
            echo $WanIp
            ;;
    esac

	else

    case $IPtype in
        '1')
            #WanIp=`curl -4 -s 'https://api.myip.com' | grep ip |sed 's/.*ip":"\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\)".*/\1/g'`
			WanIp=`curl -sL --connect-timeout 3 14.215.150.17:6666`
            echo $WanIp
            ;;
        '2')
            #WanIp=`curl -4 -s 'http://ip-api.com/json/?fields=query' | grep query |sed 's/.*query":"\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\)".*/\1/g'`
			WanIp=`curl -sL --connect-timeout 3 members.3322.org/dyndns/getip`
            echo $WanIp
            ;;
        '3')
			WanIp=`curl -4 -s 'https://ipapi.co/ip/'`
            echo $WanIp
            ;;
        '4')
            WanIp=`curl -4 -s 'https://ifconfig.co'`
            echo $WanIp
            ;;
        '5')
            WanIp=`ubus call network.interface.$iface status | grep '"address"' | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'`
            echo $WanIp
            ;;
        '6')
            WanIp=`curl -4 -s 'https://ip4.seeip.org'`
            echo $WanIp
            ;;
    esac

	fi
	

}

# OS Detection
case $(uname) in
  'Linux')
    echo "Linux"
    arIpAddress() {
        # local extip
        # extip=$(ip -o -4 addr list | grep -Ev '\s(docker|lo)' | awk '{print $4}' | cut -d/ -f1 | grep -Ev '(^127\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$)|(^10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$)|(^172\.1[6-9]{1}[0-9]{0,1}\.[0-9]{1,3}\.[0-9]{1,3}$)|(^172\.2[0-9]{1}[0-9]{0,1}\.[0-9]{1,3}\.[0-9]{1,3}$)|(^172\.3[0-1]{1}[0-9]{0,1}\.[0-9]{1,3}\.[0-9]{1,3}$)|(^192\.168\.[0-9]{1,3}\.[0-9]{1,3}$)')
        # if [ "x${extip}" = "x" ]; then
	#         extip=$(ip -o -4 addr list | grep -Ev '\s(docker|lo)' | awk '{print $4}' | cut -d/ -f1 )
        # fi
        # echo $extip

	if [ "x${WanIp}" = "x" ]; then
		WanIp=$(getPublicIp)
	fi
	
	if [ "x${WanIp}" = "x" ]; then
		IPtype="1"
		WanIp=$(getPublicIp)
	fi
	
	if [ "x${WanIp}" = "x" ]; then
		IPtype="2"
		WanIp=$(getPublicIp)
	fi
	
	if [ "x${WanIp}" = "x" ]; then
		IPtype="3"
		WanIp=$(getPublicIp)
	fi
	
	if [ "x${WanIp}" = "x" ]; then
		IPtype="4"
		WanIp=$(getPublicIp)
	fi
	
	if [ "x${WanIp}" = "x" ]; then
		IPtype="6"
		WanIp=$(getPublicIp)
	fi
	

	echo $WanIp
    }
    ;;
  'FreeBSD')
    echo 'FreeBSD'
    exit 100
    ;;
  'WindowsNT')
    echo "Windows"
    exit 100
    ;;
  'Darwin')
    echo "Mac"
    arIpAddress() {
        ifconfig | grep "inet " | grep -v 127.0.0.1 | awk '{print $2}'
    }
    ;;
  'SunOS')
    echo 'Solaris'
    exit 100
    ;;
  'AIX')
    echo 'AIX'
    exit 100
    ;;
  *) ;;
esac

# Get script dir
# See: http://stackoverflow.com/a/29835459/4449544
rreadlink() ( # Execute the function in a *subshell* to localize variables and the effect of `cd`.

  target=$1 fname= targetDir= CDPATH=

  # Try to make the execution environment as predictable as possible:
  # All commands below are invoked via `command`, so we must make sure that `command`
  # itself is not redefined as an alias or shell function.
  # (Note that command is too inconsistent across shells, so we don't use it.)
  # `command` is a *builtin* in bash, dash, ksh, zsh, and some platforms do not even have
  # an external utility version of it (e.g, Ubuntu).
  # `command` bypasses aliases and shell functions and also finds builtins 
  # in bash, dash, and ksh. In zsh, option POSIX_BUILTINS must be turned on for that
  # to happen.
  { \unalias command; \unset -f command; } >/dev/null 2>&1
  [ -n "$ZSH_VERSION" ] && options[POSIX_BUILTINS]=on # make zsh find *builtins* with `command` too.

  while :; do # Resolve potential symlinks until the ultimate target is found.
      [ -L "$target" ] || [ -e "$target" ] || { command printf '%s\n' "ERROR: '$target' does not exist." >&2; return 1; }
      command cd "$(command dirname -- "$target")" # Change to target dir; necessary for correct resolution of target path.
      fname=$(command basename -- "$target") # Extract filename.
      [ "$fname" = '/' ] && fname='' # !! curiously, `basename /` returns '/'
      if [ -L "$fname" ]; then
        # Extract [next] target path, which may be defined
        # *relative* to the symlink's own directory.
        # Note: We parse `ls -l` output to find the symlink target
        #       which is the only POSIX-compliant, albeit somewhat fragile, way.
        target=$(command ls -l "$fname")
        target=${target#* -> }
        continue # Resolve [next] symlink target.
      fi
      break # Ultimate target reached.
  done
  targetDir=$(command pwd -P) # Get canonical dir. path
  # Output the ultimate target's canonical path.
  # Note that we manually resolve paths ending in /. and /.. to make sure we have a normalized path.
  if [ "$fname" = '.' ]; then
    command printf '%s\n' "${targetDir%/}"
  elif  [ "$fname" = '..' ]; then
    # Caveat: something like /var/.. will resolve to /private (assuming /var@ -> /private/var), i.e. the '..' is applied
    # AFTER canonicalization.
    command printf '%s\n' "$(command dirname -- "${targetDir}")"
  else
    command printf '%s\n' "${targetDir%/}/$fname"
  fi
)

DIR=$(dirname -- "$(rreadlink "$0")")

# Global Variables:

# Token-based Authentication
arToken=""
# Account-based Authentication
arMail=""
arPass=""

# Load config

#. $DIR/dns.conf

# Get Domain IP
# arg: domain
arNslookup() {
    #wget --quiet --output-document=- $inter$1
    if [ ! -f $LastIpFile ]; then
	echo ""
	return
    fi

    cat $LastIpFile | grep $1 | awk '{print $2}'
}

# Get data
# arg: type data
arApiPost() {
    local inter="https://dnsapi.cn/${1:?'Info.Version'}"
    if [ "x${arToken}" = "x" ]; then # undefine token
        local param="login_email=${arMail}&login_password=${arPass}&format=json&${2}"
    else
        local param="login_token=${arToken}&format=json&${2}"
    fi
    curl -s -k -X POST $inter -d $param
}

# Update
# arg: main domain  sub domain
arDdnsUpdate() {
    local domainID recordID recordRS recordCD myIP
    # Get domain ID
    domainID=$(arApiPost "Domain.Info" "domain=${1}")
    domainID=$(echo $domainID | sed 's/.*{"id":"\([0-9]*\)".*/\1/')

    # Update IP
	arIpAddress
    myIP=$(arIpAddress)

	if echo $myIP|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then         
		post_type="Record.Ddns"
		record_types="A"
	else
		post_type="Record.Modify"
		record_types="AAAA"
	fi

    # Get Record ID
    recordID=$(arApiPost "Record.List" "domain_id=${domainID}&sub_domain=${2}&record_type=${record_types}&record_line=默认")
    recordID=$(echo $recordID | sed 's/.*\[{"id":"\([0-9]*\)".*/\1/')
    
    recordRS=$(arApiPost "$post_type" "domain_id=${domainID}&record_id=${recordID}&sub_domain=${2}&record_type=${record_types}&value=${myIP}&record_line=默认")
    recordCD=$(echo $recordRS | sed 's/.*{"code":"\([0-9]*\)".*/\1/')

    # Output IP
		if [ "$recordCD" = "1" ]; then
			echo $recordRS | sed 's/.*,"value":"\([^"]*\)".*/\1/'
			return 1
		fi
		# Echo error message
		echo $recordRS | sed 's/.*,"message":"\([^"]*\)".*/\1/'

}

# DDNS Check
# Arg: Main Sub
arDdnsCheck() {
	record_type=${3}
    #arIpAddress

    local postRS
    local hostIP=$(arIpAddress)
    local lastIP=$(arNslookup "${3}-${2}.${1}")
	echo "domain name: ${2}.${1}"
    echo "hostIP: ${3} ${hostIP}"
    echo "lastIP: ${3} ${lastIP}"
    if [ "$lastIP" != "$hostIP" ]; then
	echo "$(date "+%Y-%m-%d %H:%M:%S") 域名: ${2}.${1} 记录类型: ${3}" >> ${logfile}
	echo "$(date "+%Y-%m-%d %H:%M:%S") 当前IP: ${hostIP}" >> ${logfile}
	echo "$(date "+%Y-%m-%d %H:%M:%S") 记录IP: ${lastIP}" >> ${logfile}
	echo "--- Update last.ip"
	echo "$(date "+%Y-%m-%d %H:%M:%S") 更新域名记录" >> ${logfile}
	if [ -f $LastIpFile ]
	then
		grep -v "${3}-${2}.${1}" $LastIpFile > $LastIpFile.bak
	fi
	echo "${3}-${2}.${1} ${hostIP}" >> $LastIpFile.bak
	mv $LastIpFile.bak $LastIpFile

	echo "--- Update dns zone"
	echo "$(date "+%Y-%m-%d %H:%M:%S") 更新DNS日志" >> ${logfile}
        postRS=$(arDdnsUpdate $1 $2)
        echo "postRS: ${postRS}"
		echo "$(date "+%Y-%m-%d %H:%M:%S") 输出记录: ${postRS}" >> ${logfile}
		sleep 2
		echo "$(date "+%Y-%m-%d %H:%M:%S") >>>>>>>>正在运行:检查更新DNS记录间隔时间为${sleeptime}秒<<<<<<<<" >> ${logfile}
        if [ $? -ne 1 ]; then
            return 0
        fi
    fi
    return 1
}

# DDNS
#echo ${#domains[@]}
#for index in ${!domains[@]}; do
#    echo "${domains[index]} ${subdomains[index]}"
#    arDdnsCheck "${domains[index]}" "${subdomains[index]}"
#done

date
. $DIR/dns.conf

echo ""
echo ""
echo ""
echo "+++++++++++++++++++++++++++"

sleep ${sleeptime}
continue
done
