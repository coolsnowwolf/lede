#!/bin/bash

PATH="/usr/sbin:/usr/bin:/sbin:/bin"
binpath=$(uci get AdGuardHome.AdGuardHome.binpath)
if [ -z "$binpath" ]; then
uci set AdGuardHome.AdGuardHome.binpath="/tmp/AdGuardHome/AdGuardHome"
binpath="/tmp/AdGuardHome/AdGuardHome"
fi
mkdir -p ${binpath%/*}
upxflag=$(uci get AdGuardHome.AdGuardHome.upxflag 2>/dev/null)

check_if_already_running(){
	running_tasks="$(ps |grep "AdGuardHome" |grep "update_core" |grep -v "grep" |awk '{print $1}' |wc -l)"
	[ "${running_tasks}" -gt "2" ] && echo -e "\nA task is already running."  && EXIT 2
}

check_wgetcurl(){
	which curl && downloader="curl -L -k --retry 2 --connect-timeout 20 -o" && return
	which wget-ssl && downloader="wget-ssl --no-check-certificate -t 2 -T 20 -O" && return
	[ -z "$1" ] && opkg update || (echo error opkg && EXIT 1)
	[ -z "$1" ] && (opkg remove wget wget-nossl --force-depends ; opkg install wget ; check_wgetcurl 1 ;return)
	[ "$1" == "1" ] && (opkg install curl ; check_wgetcurl 2 ; return)
	echo error curl and wget && EXIT 1
}

check_latest_version(){
	check_wgetcurl
	latest_ver="$($downloader - https://api.github.com/repos/AdguardTeam/AdGuardHome/releases/latest 2>/dev/null|grep -E 'tag_name' |grep -E 'v[0-9.]+' -o 2>/dev/null)"
	if [ -z "${latest_ver}" ]; then
		echo -e "\nFailed to check latest version, please try again later."  && EXIT 1
	fi
	now_ver="$($binpath -c /dev/null --check-config 2>&1| grep -m 1 -E 'v[0-9.]+' -o)"
	if [ "${latest_ver}"x != "${now_ver}"x ] || [ "$1" == "force" ]; then
		echo -e "Local version: ${now_ver}., cloud version: ${latest_ver}."
		doupdate_core
	else
		echo -e "\nLocal version: ${now_ver}, cloud version: ${latest_ver}."
		echo -e "You're already using the latest version."
		if [ ! -z "$upxflag" ]; then
			filesize=$(ls -l $binpath | awk '{ print $5 }')
			if [ $filesize -gt 8000000 ]; then
				echo -e "start upx may take a long time"
				doupx
				mkdir -p "/tmp/AdGuardHomeupdate/AdGuardHome" >/dev/null 2>&1
				rm -fr /tmp/AdGuardHomeupdate/AdGuardHome/${binpath##*/}
				/tmp/upx-${upx_latest_ver}-${Arch}_linux/upx $upxflag $binpath -o /tmp/AdGuardHomeupdate/AdGuardHome/${binpath##*/}
				rm -rf /tmp/upx-${upx_latest_ver}-${Arch}_linux
				/etc/init.d/AdGuardHome stop nobackup
				rm $binpath
				mv -f /tmp/AdGuardHomeupdate/AdGuardHome/${binpath##*/} $binpath
				/etc/init.d/AdGuardHome start
				echo -e "finished"
			fi
		fi
		EXIT 0
	fi
}

doupx(){
	Archt="$(opkg info kernel | grep Architecture | awk -F "[ _]" '{print($2)}')"
	case $Archt in
	"i386")
		Arch="i386"
		;;
	"i686")
		Arch="i386"
		echo -e "i686 use $Arch may have bug"
		;;
	"x86")
		Arch="amd64"
		;;
	"mipsel")
		Arch="mipsel"
		;;
	"mips64el")
		Arch="mips64el"
		Arch="mipsel"
		echo -e "mips64el use $Arch may have bug"
		;;
	"mips")
		Arch="mips"
		;;
	"mips64")
		Arch="mips64"
		Arch="mips"
		echo -e "mips64 use $Arch may have bug"
		;;
	"arm")
		Arch="arm"
		;;
	"armeb")
		Arch="armeb"
		;;
	"aarch64")
		Arch="arm64"
		;;
	"powerpc")
		Arch="powerpc"
		;;
	"powerpc64")
		Arch="powerpc64"
		;;
	*)
		echo -e "error not support $Archt if you can use offical release please issue a bug"
		EXIT 1
		;;
	esac
	upx_latest_ver="$($downloader - https://api.github.com/repos/upx/upx/releases/latest 2>/dev/null|grep -E 'tag_name' |grep -E '[0-9.]+' -o 2>/dev/null)"
	$downloader /tmp/upx-${upx_latest_ver}-${Arch}_linux.tar.xz "https://github.com/upx/upx/releases/download/v${upx_latest_ver}/upx-${upx_latest_ver}-${Arch}_linux.tar.xz" 2>&1
	#tar xvJf
	which xz || (opkg list | grep ^xz || opkg update && opkg install xz) || (echo "xz download fail" && EXIT 1)
	mkdir -p /tmp/upx-${upx_latest_ver}-${Arch}_linux
	xz -d -c /tmp/upx-${upx_latest_ver}-${Arch}_linux.tar.xz| tar -x -C "/tmp" >/dev/null 2>&1
	if [ ! -e "/tmp/upx-${upx_latest_ver}-${Arch}_linux/upx" ]; then
		echo -e "Failed to download upx."
		EXIT 1
	fi
	rm /tmp/upx-${upx_latest_ver}-${Arch}_linux.tar.xz
}

doupdate_core(){
	echo -e "Updating core..."
	mkdir -p "/tmp/AdGuardHomeupdate"
	rm -rf /tmp/AdGuardHomeupdate/* >/dev/null 2>&1
	Archt="$(opkg info kernel | grep Architecture | awk -F "[ _]" '{print($2)}')"
	case $Archt in
	"i386")
		Arch="386"
		;;
	"i686")
		Arch="386"
		;;
	"x86")
		Arch="amd64"
		;;
	"mipsel")
		Arch="mipsle"
		;;
	"mips64el")
		Arch="mips64le"
		Arch="mipsle"
		echo -e "mips64el use $Arch may have bug"
		;;
	"mips")
		Arch="mips"
		;;
	"mips64")
		Arch="mips64"
		Arch="mips"
		echo -e "mips64 use $Arch may have bug"
		;;
	"arm")
		Arch="arm"
		;;
	"aarch64")
		Arch="arm64"
		;;
	"powerpc")
		Arch="ppc"
		echo -e "error not support $Archt"
		EXIT 1
		;;
	"powerpc64")
		Arch="ppc64"
		echo -e "error not support $Archt"
		EXIT 1
		;;
	*)
		echo -e "error not support $Archt if you can use offical release please issue a bug"
		EXIT 1
		;;
	esac
	echo -e "start download"
	grep -v "^#" /usr/share/AdGuardHome/links.txt >/tmp/run/AdHlinks.txt
	while read link
	do
		eval link="$link"
		$downloader /tmp/AdGuardHomeupdate/${link##*/} "$link" 2>&1
		if [ "$?" != "0" ]; then
			echo "download failed try another download"
			rm -f /tmp/AdGuardHomeupdate/${link##*/}
		else
			local success="1"
			break
		fi
	done < "/tmp/run/AdHlinks.txt"
	rm /tmp/run/AdHlinks.txt
	[ -z "$success" ] && echo "no download success" && EXIT 1
	if [ "${link##*.}" == "gz" ]; then
		tar -zxf "/tmp/AdGuardHomeupdate/${link##*/}" -C "/tmp/AdGuardHomeupdate/"
		if [ ! -e "/tmp/AdGuardHomeupdate/AdGuardHome" ]; then
			echo -e "Failed to download core."
			rm -rf "/tmp/AdGuardHomeupdate" >/dev/null 2>&1
			EXIT 1
		fi
		downloadbin="/tmp/AdGuardHomeupdate/AdGuardHome/AdGuardHome"
	else
		downloadbin="/tmp/AdGuardHomeupdate/${link##*/}"
	fi
	chmod 755 $downloadbin
	echo -e "download success start copy"
	if [ -n "$upxflag" ]; then
		echo -e "start upx may take a long time"
		doupx
		/tmp/upx-${upx_latest_ver}-${Arch}_linux/upx $upxflag $downloadbin
		rm -rf /tmp/upx-${upx_latest_ver}-${Arch}_linux
	fi
	echo -e "start copy"
	/etc/init.d/AdGuardHome stop nobackup
	rm "$binpath"
	mv -f "$downloadbin" "$binpath"
	if [ "$?" == "1" ]; then
		echo "mv failed maybe not enough space please use upx or change bin to /tmp/AdGuardHome"
		EXIT 1
	fi
	/etc/init.d/AdGuardHome start
	rm -rf "/tmp/AdGuardHomeupdate" >/dev/null 2>&1
	echo -e "Succeeded in updating core."
	echo -e "Local version: ${latest_ver}, cloud version: ${latest_ver}.\n"
	EXIT 0
}

EXIT(){
	rm /var/run/update_core 2>/dev/null
	[ "$1" != "0" ] && touch /var/run/update_core_error
	exit $1
}

main(){
	check_if_already_running
	check_latest_version $1
}
	trap "EXIT 1" SIGTERM SIGINT
	touch /var/run/update_core
	rm /var/run/update_core_error 2>/dev/null
	main $1
