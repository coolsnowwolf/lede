#!/bin/sh

unify_ps_status() {
	if [ "$(ps --version 2>&1 |grep -c procps-ng)" -eq 1 ];then
		echo "$(ps -ef |grep -v grep |grep -c "$1")"
	else
		echo "$(ps |grep -v grep |grep -c "$1")"
	fi
}

unify_ps_pids() {
	if [ "$(ps --version 2>&1 |grep -c procps-ng)" -eq 1 ];then
		echo "$(ps -ef |grep "$1" |grep -v grep |awk '{print $2}' 2>/dev/null)"
	else
		echo "$(ps |grep "$1" |grep -v grep |awk '{print $1}' 2>/dev/null)"
	fi
}

unify_ps_prevent() {
	if [ "$(ps --version 2>&1 |grep -c procps-ng)" -eq 1 ];then
		echo "$(ps -ef |grep -v openclash_watchdog |grep -c openclash.sh 2>/dev/null)"
	else
		echo "$(ps |grep -v openclash_watchdog |grep -c openclash.sh 2>/dev/null)"
	fi
}

unify_ps_cfgname() {
	if [ "$(ps --version 2>&1 |grep -c procps-ng)" -eq 1 ];then
		echo "$(ps -efw |grep /etc/openclash/clash 2>/dev/null |grep -v grep |awk -F '-f ' '{print $2}' 2>/dev/null)"
	else
		echo "$(ps -w |grep /etc/openclash/clash 2>/dev/null |grep -v grep |awk -F '-f ' '{print $2}' 2>/dev/null)"
	fi
}