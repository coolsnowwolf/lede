#!/bin/sh

function check_if_already_running(){
	running_tasks="$(ps |grep "unblockneteasemusic" |grep "update_core" |grep -v "grep" |awk '{print $1}' |wc -l)"
	[ "${running_tasks}" -gt "2" ] && echo -e "\nA task is already running." >>/tmp/unblockmusic_update.log && exit 2
}

function clean_log(){
	echo "" > /tmp/unblockmusic_update.log
}

function check_latest_version(){
	latest_ver="$(wget-ssl --no-check-certificate -O- https://github.com/nondanee/UnblockNeteaseMusic/commits/master |tr -d '\n' |grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g')"
	[ -z "${latest_ver}" ] && echo -e "\nFailed to check latest version, please try again later." >>/tmp/unblockmusic_update.log && exit 1
	if [ ! -e "/usr/share/UnblockNeteaseMusic/local_ver" ]; then
		clean_log
		echo -e "Local version: NOT FOUND, cloud version: ${latest_ver}." >>/tmp/unblockmusic_update.log
		update_core
	else
		if [ "$(cat /usr/share/UnblockNeteaseMusic/local_ver)" != "${latest_ver}" ]; then
			clean_log
			echo -e "Local version: $(cat /usr/share/UnblockNeteaseMusic/local_ver 2>/dev/null), cloud version: ${latest_ver}." >>/tmp/unblockmusic_update.log
			update_core
		else
			echo -e "\nLocal version: $(cat /usr/share/UnblockNeteaseMusic/local_ver 2>/dev/null), cloud version: ${latest_ver}." >>/tmp/unblockmusic_update.log
			echo -e "You're already using the latest version." >>/tmp/unblockmusic_update.log
			[ "${luci_update}" == "n" ] && /etc/init.d/unblockmusic restart
			exit 3
		fi
	fi
}

function update_core(){
	echo -e "Updating core..." >>/tmp/unblockmusic_update.log

	mkdir -p "/tmp/unblockneteasemusic/core" >/dev/null 2>&1
	rm -rf /tmp/unblockneteasemusic/core/* >/dev/null 2>&1

	wget-ssl --no-check-certificate -t 1 -T 10 -O  /tmp/unblockneteasemusic/core/core.tar.gz "https://github.com/nondanee/UnblockNeteaseMusic/archive/master.tar.gz"  >/dev/null 2>&1
	tar -zxf "/tmp/unblockneteasemusic/core/core.tar.gz" -C "/tmp/unblockneteasemusic/core/" >/dev/null 2>&1
	if [ -e "/usr/share/UnblockNeteaseMusic/ca.crt" ] && [ -e "/usr/share/UnblockNeteaseMusic/server.crt" ] && [ -e "/usr/share/UnblockNeteaseMusic/server.key" ] ; then
		rm -f /tmp/unblockneteasemusic/core/UnblockNeteaseMusic-master/ca.crt /tmp/unblockneteasemusic/core/UnblockNeteaseMusic-master/server.crt /tmp/unblockneteasemusic/core/UnblockNeteaseMusic-master/server.key
	fi
	cp -a /tmp/unblockneteasemusic/core/UnblockNeteaseMusic-master/* "/usr/share/UnblockNeteaseMusic/"
	rm -rf "/tmp/unblockneteasemusic" >/dev/null 2>&1

	if [ ! -e "/usr/share/UnblockNeteaseMusic/app.js" ]; then
		echo -e "Failed to download core." >>/tmp/unblockmusic_update.log
		exit 1
	else
		echo -e "${latest_ver}" > /usr/share/UnblockNeteaseMusic/local_ver
		cat /usr/share/UnblockNeteaseMusic/package.json | grep version |awk -F ':' '{print $2}' | cut -c3-8 > /usr/share/UnblockNeteaseMusic/core_ver
	fi

	echo -e "Succeeded in updating core." >/tmp/unblockmusic_update.log
	echo -e "Local version: $(cat /usr/share/UnblockNeteaseMusic/local_ver 2>/dev/null), cloud version: ${latest_ver}.\n" >>/tmp/unblockmusic_update.log
	
	/etc/init.d/unblockmusic restart
}

function main(){
	check_if_already_running
	check_latest_version
}

  luci_update="n"
	[ "$1" == "luci_update" ] && luci_update="y"
	main
	
