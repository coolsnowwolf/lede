#!/bin/bash
# [CTCGFW]Project-OpenWrt
# Use it under GPLv3, please.
# --------------------------------------------------------
# Update repo info

function set_config(){
	git config "$1" "$2"
}

function update_18_06(){
	set_config "remote.origin.url" "https://github.com/project-openwrt/openwrt"
	set_config "branch.master.merge" "refs/heads/openwrt-18.06"
	set_config "branch.dev.merge" "refs/heads/openwrt-18.06-dev"
	set_config "remote.origin.fetch" "+refs/heads/openwrt-18.06*:refs/remotes/origin/openwrt-18.06*"
	git pull
	./scripts/feeds update -a
	./scripts/feeds install -a
}

function update_19_07(){
	set_config "remote.origin.url" "https://github.com/project-openwrt/openwrt"
	set_config "branch.master.merge" "refs/heads/openwrt-19.07"
	set_config "remote.origin.fetch" "+refs/heads/openwrt-19.07:refs/remotes/origin/openwrt-19.07"
	git pull
	./scripts/feeds update -a
	./scripts/feeds install -a
}

function update_latest(){
	set_config "remote.origin.url" "https://github.com/project-openwrt/openwrt"
	set_config "remote.origin.fetch" "+refs/heads/master:refs/remotes/origin/master"
	git pull
	./scripts/feeds update -a
	./scripts/feeds install -a
}

case "$1" in
	"18.06")
	update_18_06
	;;
	"19.07")
	update_19_07
	;;
	"latest")
	update_latest
	;;
	*)
	echo -e "Usage: $0 [18.06|19.07|latest]"
	;;
esac
